#pragma once
#include <cstdint>
#include <deque>
#include <memory>

// A stable deque implementation
template <typename T, typename Allocator = std::allocator<T>>
class old_stable_deque
{
    struct Node
    {
        T data;
        // I just don't want to have to think about
        // subtracing from size_t, so int64_t it is!
        int64_t pos_in_nodes;
    };

    using NodePAllocator = std::allocator_traits<Allocator>::template rebind_alloc<Node *>;
    using NodePAllocatorTraits = std::allocator_traits<NodePAllocator>;

    using NodeAllocator = std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using NodeAllocatorTraits = std::allocator_traits<NodeAllocator>;

    using NodesDeque = std::deque<Node *, NodePAllocator>;

    class iterator
    {
        friend class old_stable_deque;

        // Unlike a stable_vector, we need one additional pointer to store the deque that
        // contains our actual `Node` data.
        // This is required since unlike a stable_vector which has its `Node`'s stored in
        // a contiguous vector (pointer arithmetic iterates to the next node), a old_stable_deque
        // is backed by a non-contiguous container (a deque).
        // Therefore we require an additional pointer so that we have a way to fetch "the next node".
        NodesDeque &nodes_ref;
        Node *node;

        iterator(Node *node, NodesDeque &nodes_ref) : node(node), nodes_ref(nodes_ref)
        {
        }

    public:
        iterator(const iterator &iter) : nodes_ref(iter.nodes_ref), node(iter.node)
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

        T &operator[](int64_t offset) const
        {
            return nodes_ref[node->pos_in_nodes + offset]->data;
        }

        iterator &operator+=(int64_t offset)
        {
            node = nodes_ref[node->pos_in_nodes + offset];
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

        friend int64_t operator-(const iterator &left, const iterator &right)
        {
            return left.node->pos_in_nodes - right.node->pos_in_nodes;
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
            return l.node->pos_in_nodes < r.node->pos_in_nodes;
        }

        friend bool operator<=(const iterator &l, const iterator &r)
        {
            return l.node->pos_in_nodes <= r.node->pos_in_nodes;
        }

        friend bool operator>(const iterator &l, const iterator &r)
        {
            return l.node->pos_in_nodes > r.node->pos_in_nodes;
        }

        friend bool operator>=(const iterator &l, const iterator &r)
        {
            return l.node->pos_in_nodes >= r.node->pos_in_nodes;
        }
    };

    template <int howMuchToMove>
    void fix_up_pointers(iterator iter, iterator end)
    {
        while (true)
        {
            iter.node->pos_in_nodes += howMuchToMove;
            if (iter == end)
                return;
            iter++;
        }
    }

    NodeAllocator nodeAllocator;
    NodesDeque nodes;

    void shared_init()
    {
        // add end node
        Node *data = NodeAllocatorTraits::allocate(nodeAllocator, 1);
        NodeAllocatorTraits::construct(
            nodeAllocator,
            data,
            *((T *)alloca(sizeof(T))), // garbage memory
            nodes.size());
        nodes.push_back(data);
    }

public:
    old_stable_deque()
    {
        nodeAllocator = NodeAllocator();
        nodes = NodesDeque(NodePAllocator());
        shared_init();
    }
    old_stable_deque(const Allocator &allocator)
    {
        nodeAllocator = allocator;
        nodes = NodesDeque(allocator);
        shared_init();
    }
    ~old_stable_deque()
    {
        for (auto *node : nodes)
        {
            NodeAllocatorTraits::destroy(nodeAllocator, node);
            NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
        }
        nodes.clear();
    }

    iterator begin()
    {
        return iterator(nodes[0], nodes);
    }

    iterator end()
    {
        return iterator(nodes.back(), nodes);
    }

    std::size_t size()
    {
        // -1 for end() node
        return nodes.size() - 1;
    }

    void push_back(const T &value)
    {
        insert(end(), value);
    }

    void push_front(const T &value)
    {
        Node *data = NodeAllocatorTraits::allocate(nodeAllocator, 1);
        NodeAllocatorTraits::construct(nodeAllocator, data, value, 0);
        nodes.insert(nodes.begin(), data);
        fix_up_pointers<1>(begin() + 1, end());
    }

    void insert(iterator iterator, const T &value)
    {
        Node *data = NodeAllocatorTraits::allocate(nodeAllocator, 1);
        NodeAllocatorTraits::construct(nodeAllocator, data, value, iterator.node->pos_in_nodes);
        nodes.insert(nodes.begin() + iterator.node->pos_in_nodes, data);
        fix_up_pointers<1>(iterator, end());
    }

    void erase(iterator iterator)
    {
        Node *node = iterator.node;
        auto nextIter = iterator + 1;
        nodes.erase(nodes.begin() + node->pos_in_nodes);
        NodeAllocatorTraits::destroy(nodeAllocator, node);
        NodeAllocatorTraits::deallocate(nodeAllocator, node, 1);
        fix_up_pointers<-1>(nextIter, end());
    }
    T &operator[](int64_t index)
    {
        assert(index >= 0);
        return nodes[index]->data;
    }
};