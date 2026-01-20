#pragma once
#include <cstdint>
#include <deque>
#include <memory>

#define ASSERT_BREAK(condition) \
	{                           \
		if (!(condition))       \
		{                       \
			__debugbreak();     \
			assert(false);      \
		}                       \
	}

/*
What a deque is needed for:
* Fast front/back erase/insert
* O(1) random access
* Middle insert/erase can be slow, doesn't really
  matter since we primarily use a deque as a double
  sided queue.

This implmentation hits all of these marks... most of the time...

What this implementation does that is special:
* The core of the structure is like a boost::stable_vector
  (indirection stores each node location)
* The backing data structure is a deque instead of a vector
* Instead of fixing 'up pointers' (refer to `stable_vector`'s
  implementation), we use a 'middle' index to split the deque
  into two halves (a left and right). This enables us to avoid
  requiring to do an expensive iteration-pass every `push_front`.
  This is helpful for a deque since only `.*_front`/`.*_back`
  operations are expected to be fast (and used commonly).
  This also means that erasing the ends is normally fast as well
  (caveats are explained below).

Limitations:
* Inserting/erasing in the middle of the deque is O(n) due to 'up pointer' fixing
  (but this is expected for a stable_deque/stable_vector)
* If erasing more than a side pushes, performance degrades from O(1) to O(n)
  due to 'up pointer' fixing starting to occur (`begin()` starts returning the starting
  half of the 'right' side, so during every deletion we need to fix the 'up pointer' of
  every element on the right-side).

Can this be improved? Probably.
* Removing the `if` hacks would be a good start.
* Focusing on how to prevent the current limitation of erasure degrading to O(n) would be
  a priority as well.
* A .natvis to help visualize and debug the structure
*/

template <typename T, typename Allocator = std::allocator<T>>
class stable_deque
{
	struct Node
	{
		T data;
		int64_t pos;
	};

	using NodePAllocator = std::allocator_traits<Allocator>::template rebind_alloc<Node *>;
	using NodePAllocatorTraits = std::allocator_traits<NodePAllocator>;

	using NodeAllocator = std::allocator_traits<Allocator>::template rebind_alloc<Node>;
	using NodeAllocatorTraits = std::allocator_traits<NodeAllocator>;
	NodeAllocator nodeAllocator;

	struct stable_deque_data
	{
		int64_t middle = -1;

		/// Data is stored in this order (relative to the provided iterator):
		///[begin(), middle](middle, end())
		std::deque<Node *, NodePAllocator> data;
	} nodeData;

	class iterator
	{
		friend class stable_deque;

		/// Context/parent
		stable_deque_data &nodeDataRef;

		/// Boolean controlling if we are an iterator on the 'left' or 'right' side
		/// (left or right is assuming stable_deque_data::data is visualized linearly)
		bool isLeft;

		/// Current node our iterator is operating on
		Node *node;

		iterator(stable_deque_data &nodeDataRef, bool isLeft, Node *node) : nodeDataRef(nodeDataRef), isLeft(isLeft), node(node)
		{
		}

		decltype(stable_deque_data::data)::iterator get_underlying_data_iterator() const
		{
			if (isLeft)
				return nodeDataRef.data.begin() + (nodeDataRef.middle - node->pos);
			else
				return nodeDataRef.data.begin() + (nodeDataRef.middle + 1 + node->pos);
		}

	public:
		iterator(const iterator &iter) : nodeDataRef(iter.nodeDataRef), isLeft(iter.isLeft), node(iter.node)
		{
		}

		T &operator*() const
		{
			return node->data;
		}

		// Increment / Decrement
		iterator &operator++()
		{
			*this += 1;
			return *this;
		}

		iterator &operator++(int)
		{
			*this += 1;
			return *this;
		}

		iterator &operator--()
		{
			*this -= 1;
			return *this;
		}

		iterator &operator--(int)
		{
			*this -= 1;
			return *this;
		}

		/// Positive offset means adding an element closer to the "right" side
		iterator &operator+=(int64_t offset)
		{
			int64_t locationToIndex;
			if (isLeft)
			{
				locationToIndex = node->pos - offset;

				// Switch sides
				if (locationToIndex < 0)
					this->isLeft = !this->isLeft;
				this->node = nodeDataRef.data[nodeDataRef.middle - locationToIndex];
			}
			else
			{
				locationToIndex = node->pos + offset;

				// Switch sides
				if (locationToIndex < 0)
					this->isLeft = !this->isLeft;
				this->node = nodeDataRef.data[nodeDataRef.middle + 1 + locationToIndex];
			}
			return *this;
		}

		iterator &operator-=(int64_t offset)
		{
			*this += -offset;
			return *this;
		}

		friend iterator operator+(const iterator &left, int64_t offset)
		{
			iterator tmp(left);
			tmp += offset;
			return tmp;
		}

		friend iterator operator+(int64_t offset, const iterator &right)
		{
			iterator tmp(right);
			tmp += offset;
			return tmp;
		}

		friend iterator operator-(const iterator &left, int64_t offset)
		{
			iterator tmp(left);
			tmp -= offset;
			return tmp;
		}

		// Comparison operators
		friend bool operator==(const iterator &l, const iterator &r)
		{
			return l.node == r.node;
		}

		friend bool operator!=(const iterator &l, const iterator &r)
		{
			return l.node != r.node;
		}

		friend bool operator<(const iterator &l, const iterator &r)
		{
			return l.isLeft == true && r.isLeft == false || l.node->pos < r.node->pos && l.isLeft == r.isLeft;
		}

		friend bool operator<=(const iterator &l, const iterator &r)
		{
			return l.isLeft == true && r.isLeft == false || l.node->pos <= r.node->pos && l.isLeft == r.isLeft;
		}

		friend bool operator>(const iterator &l, const iterator &r)
		{
			return !(l.node->pos < r.node->pos) && l != r;
		}

		friend bool operator>=(const iterator &l, const iterator &r)
		{
			return !(l.node->pos < r.node->pos) || l == r;
		}

		// Other
		T &operator[](int64_t offset) const
		{
			return *(*this + offset);
		}
	};

	// Completes a `amountToShiftEachPointer` for the direction `directionToIterate`.
	// `amountToShiftEachPointer` is inclusive to `end`.
	template <int amountToIterateTheFixingLoop, int amountToShiftEachPointer>
	void fix_up_pointers(iterator iter, iterator end)
	{
		while (true)
		{
			iter.node->pos += amountToShiftEachPointer;
			if (iter == end)
				return;
			iter += amountToIterateTheFixingLoop;
		}
	}

	void shared_init()
	{
		// Add end node
		Node *newNode = NodeAllocatorTraits::allocate(nodeAllocator, 1);
		NodeAllocatorTraits::construct(
			nodeAllocator,
			newNode,
			*((T *)alloca(sizeof(T))), // Garbage memory
			0);
		nodeData.data.push_back(newNode);
	}

	enum class InsertInnerOptions
	{
		None,
		ForceLeft,
		ForceRight
	};

	// Insert on the left side of the deque
	void insert_left(const iterator &iter, const T &value)
	{
		if (nodeData.middle == -1) [[unlikely]]
		{
			Node *newNode = NodeAllocatorTraits::allocate(nodeAllocator, 1);
			NodeAllocatorTraits::construct(nodeAllocator, newNode, value, nodeData.middle);
			nodeData.data.insert(iter.get_underlying_data_iterator(), newNode);
			nodeData.middle += 1;
			fix_up_pointers<1, 1>(iter - 1, begin());
		}
		else
		{
			Node *newNode = NodeAllocatorTraits::allocate(nodeAllocator, 1);
			NodeAllocatorTraits::construct(nodeAllocator, newNode, value, iter.node->pos);
			nodeData.data.insert(iter.get_underlying_data_iterator(), newNode);
			nodeData.middle += 1;
			fix_up_pointers<1, 1>(iter - 1, begin());
		}
	}

	void insert_right(const iterator &iter, const T &value)
	{
		if (nodeData.data.back()->pos == 0) [[unlikely]]
		{
			Node *newNode = NodeAllocatorTraits::allocate(nodeAllocator, 1);
			NodeAllocatorTraits::construct(nodeAllocator, newNode, value, iter.node->pos);
			nodeData.data.insert(iter.get_underlying_data_iterator(), newNode);
			fix_up_pointers<1, 1>(iter, end());
		}
		else
		{
			Node *newNode = NodeAllocatorTraits::allocate(nodeAllocator, 1);
			NodeAllocatorTraits::construct(nodeAllocator, newNode, value, iter.node->pos);
			nodeData.data.insert(iter.get_underlying_data_iterator(), newNode);
			fix_up_pointers<1, 1>(iter, end());
		}
	}
	template <InsertInnerOptions options>
	void insert_inner(const iterator &iter, const T &value)
	{
		if constexpr (options == InsertInnerOptions::ForceLeft)
		{
			insert_left(iter, value);
		}
		else if constexpr (options == InsertInnerOptions::ForceRight)
		{
			insert_right(iter, value);
		}
		else
		{
			// Choose the side in this order:
			// 1. If iter is `pos == 0 && middle == -1`, use left side.
			//    This ensures that the LHS always has at-least 1 element (so that when we use
			//    `insert(begin(), ...)` we add to the LHS).
			// 2. If iter is not `pos == 0` (border between left/right), use the same side of `iter.isLeft`

			if (iter.node->pos == 0 && nodeData.middle == -1) [[unlikely]]
			{
				insert_left(iter, value);
			}
			else
			{
				if (iter.isLeft)
					insert_left(iter, value);
				else
					insert_right(iter, value);
			}
		}
	}

public:
	stable_deque()
	{
		nodeAllocator = NodeAllocator();
		shared_init();
	}

	~stable_deque()
	{
		for (auto *node : nodeData.data)
		{
			NodeAllocatorTraits::destroy(nodeAllocator, node);
			NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
		}
	}

	iterator begin()
	{
		return iterator(nodeData, nodeData.middle != -1, nodeData.data.front());
	}

	iterator end()
	{
		return iterator(nodeData, false, nodeData.data.back());
	}

	std::size_t size()
	{
		// -1 for end() node
		return nodeData.data.size() - 1;
	}

	void push_back(const T &value)
	{
		// Add to 'right' of the 'middle' of our deque
		insert_inner<InsertInnerOptions::ForceRight>(end(), value);
	}

	void push_front(const T &value)
	{
		// Add to 'left' of the 'middle' of our deque
		insert_inner<InsertInnerOptions::ForceLeft>(begin(), value);
	}

	void insert(iterator iterator, const T &value)
	{
		insert_inner<InsertInnerOptions::None>(iterator, value);
	}

	void erase(iterator iterator)
	{
		typename decltype(stable_deque_data::data)::iterator underlyingNode;
		if (iterator.isLeft)
		{
			nodeData.middle -= 1;
			// Since we subtract one for every iterator, we need to iterate two every loop
			fix_up_pointers<2, -1>(iterator, begin());
			underlyingNode = iterator.get_underlying_data_iterator();
		}
		else
		{
			underlyingNode = iterator.get_underlying_data_iterator();
			fix_up_pointers<2, -1>(iterator, end());
		}
		Node *node = iterator.node;
		nodeData.data.erase(underlyingNode);
		NodeAllocatorTraits::destroy(nodeAllocator, node);
		NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
	}
	T &operator[](int64_t index)
	{
		ASSERT_BREAK(index >= 0 && index < end().node->pos + nodeData.middle + 1);
		return *(begin() + index);
	}
};