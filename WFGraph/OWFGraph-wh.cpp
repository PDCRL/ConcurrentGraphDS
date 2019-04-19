#include <iostream>
#include <climits>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <cmath>
#include <fstream>
#include <sys/time.h>

#include <array>

using namespace std;

enum op_type
{
    insert_vertex,
    insert_edge,
    search_delete_vertex,
    execute_delete_vertex,
    determine_delete_vertex,
    search_delete_edge,
    execute_delete_edge,
    determine_delete_edge,
    search_vertex,
    search_edge,
    success,
    failure,
    contains_vertex,
    contains_edge,
    update_approximation
};

typedef struct Node
{
    int data;
    atomic<struct Node *> v_next;
    atomic<struct Node *> e_next;
    atomic<struct Node *> point_v;
    atomic<bool> mark;
} Node_List;

typedef struct OpDesc
{
    long int phase;
    op_type type;
    Node_List *node;
    Node_List *dnode;
    Node_List *prev;
    Node_List *curr;
    atomic<bool> mark;
} OpDesc;

typedef struct
{
    int secs;
    int usecs;
} TIME_DIFF;

TIME_DIFF *my_difftime(struct timeval *start, struct timeval *end)
{
    TIME_DIFF *diff = (TIME_DIFF *)malloc(sizeof(TIME_DIFF));
    if (start->tv_sec == end->tv_sec)
    {
        diff->secs = 0;
        diff->usecs = end->tv_usec - start->tv_usec;
    }
    else
    {
        diff->usecs = 1000000 - start->tv_usec;
        diff->secs = end->tv_sec - (start->tv_sec + 1);
        diff->usecs += end->tv_usec;
        if (diff->usecs >= 1000000)
        {
            diff->usecs -= 1000000;
            diff->secs += 1;
        }
    }
    return diff;
}

int is_marked_ref(long i)
{
    return (int)(i & 0x1L);
}

long unset_mark(long i)
{
    i &= ~0x1L;
    return i;
}

long set_mark(long i)
{
    i |= 0x1L;
    return i;
}

long get_unmarked_ref(long w)
{
    return w & ~0x1L;
}

long get_marked_ref(long w)
{
    return w | 0x1L;
}

int size;
long int MAX_FAILURES = 20;
std::atomic<OpDesc *> *state;

Node_List *head;
Node_List *tail;

class wait_free_graph
{
  private:
    atomic<long int> max_phase;
    long int MAX_ERROR;

  public:
    wait_free_graph(int no_of_threads)
    {
        size = no_of_threads;
        head = new Node_List;
        tail = new Node_List;
        head->data = INT_MIN;
        head->mark = false;
        tail->data = INT_MAX;
        head->v_next = tail;
        head->e_next = NULL;
        tail->mark = false;
        tail->v_next = NULL;
        tail->e_next = NULL;
        max_phase.store(0, std::memory_order_seq_cst);
        state = new atomic<OpDesc *>[size];
        for (int i = 0; i < size; i++)
        {
            state[i].store(init_op(0, success, NULL, NULL, NULL, NULL));
        }
    }

    bool insert_node(int id, int key, int dest, bool is_vertex);
    bool delete_node(int id, int key, int dest, bool is_vertex);
    bool contain_node(int id, int key, int dest, bool is_vertex);
    void help_insert_vertex(int id, long int phase);
    void help_insert_edge(int id, long int phase);
    void help_delete_vertex(int id, long int phase);
    void help_delete_edge(int id, long int phase);
    void help_contains_vertex(int id, long int phase);
    void help_contains_edge(int id, long int phase);
    void print_graph();
    Node_List *create_vertex(int key);
    Node_List *create_edge(int key);
    OpDesc *init_op(long int phase, op_type type, Node_List *node, Node_List *dnode, Node_List *prev, Node_List *curr);
    long int next_phase();

    bool fast_insert_vertex(int id, int key);
    bool fast_insert_edge(int id, int key, int dest);
    bool fast_delete_vertex(int id, int key);
    bool fast_delete_edge(int id, int key, int dest);
    bool fast_contains_vertex(int id, int key);
    bool fast_contains_edge(int id, int key1, int key2);
    void locate_eplus(Node_List *startE, Node_List **n1, Node_List **n2, int key);
    bool contains_vplus(Node_List **n1, Node_List **n2, int key1, int key2);
    void locate_vplus(Node_List *startV, Node_List **n1, Node_List **n2, int key);
    void locate_vplusplus(Node_List *startV, Node_List **n1, Node_List **n2, int key);
    bool contain_cplus(Node_List **n1, Node_List **n2, int key1, int key2);
    void locate_cplus(Node_List *startV, Node_List **n1, Node_List **n2, int key);
};

OpDesc *wait_free_graph ::init_op(long int phase, op_type type, Node_List *node, Node_List *dnode, Node_List *prev, Node_List *curr)
{
    while (true)
    {
        try
        {
            OpDesc *op = new OpDesc;
            op->phase = phase;
            op->type = type;
            op->node = node;
            op->dnode = dnode;
            op->prev = prev;
            op->curr = curr;
            return op;
        }
        catch (string error)
        {
            continue;
        }
    }
}

void wait_free_graph ::locate_cplus(Node_List *startV, Node_List **n1, Node_List **n2, int key)
{
    Node_List *currv, *predv;
    predv = startV;
    currv = (Node_List *)get_unmarked_ref((long)predv->v_next.load());
    while (currv && currv->data < key)
    {

        predv = currv;
        currv = (Node_List *)get_unmarked_ref((long)currv->v_next.load());
    }

    (*n1) = predv;
    (*n2) = currv;
    return;
}

bool wait_free_graph ::contain_cplus(Node_List **n1, Node_List **n2, int key1, int key2)
{
    Node_List *curr1, *pred1, *curr2, *pred2;
    if (key1 < key2)
    {

        locate_cplus(head, &pred1, &curr1, key1); //first look for key1
        if ((!curr1->v_next.load()) || curr1->data != key1)
            return false; // key1 is not present in the vertex-list

        locate_cplus(curr1, &pred2, &curr2, key2); // looking for key2 only if key1 present
        if ((!curr2->v_next.load()) || curr2->data != key2)
            return false; // key2 is not present in the vertex-list
    }
    else
    {
        locate_cplus(head, &pred2, &curr2, key2); //first look for key2
        if ((!curr2->v_next.load()) || curr2->data != key2)
            return false; // key2 is not present in the vertex-list

        locate_cplus(curr2, &pred1, &curr1, key1); // looking for key1 only if key2 present
        if ((!curr1->v_next.load()) || curr1->data != key1)
            return false; // key1 is not present in the vertex-list
    }
    (*n1) = curr1;
    (*n2) = curr2;
    return true;
}

void wait_free_graph ::locate_vplus(Node_List *startV, Node_List **n1, Node_List **n2, int key)
{
    Node_List *succv, *currv, *predv;
retry:
    while (true)
    {
        predv = startV;
        currv = predv->v_next.load();
        while (true)
        {
            succv = currv->v_next.load();
            while (currv && currv->v_next != NULL && is_marked_ref((long)succv) && currv->data < key)
            {
                if (!predv->v_next.compare_exchange_strong(currv, (Node_List *)get_unmarked_ref((long)succv), memory_order_seq_cst))
                    goto retry;
                currv = (Node_List *)get_unmarked_ref((long)succv);
                succv = currv->v_next.load();
            }
            if (currv->data >= key)
            {
                (*n1) = predv;
                (*n2) = currv;
                return;
            }
            predv = currv;
            currv = (Node_List *)get_unmarked_ref((long)succv);
        }
    }
}

void wait_free_graph ::locate_vplusplus(Node_List *startV, Node_List **n1, Node_List **n2, int key)
{
    Node_List *succv, *currv, *predv;

    predv = startV;
    currv = (Node_List *)get_unmarked_ref((long)predv->v_next.load());
    while (currv && currv->v_next && currv->data < key)
    {

        predv = currv;
        currv = (Node_List *)get_unmarked_ref((long)currv->v_next.load());
    }

    (*n1) = predv;
    (*n2) = currv;
    return;
}

bool wait_free_graph ::contains_vplus(Node_List **n1, Node_List **n2, int key1, int key2)
{
    Node_List *curr1, *pred1, *curr2, *pred2;
    if (key1 < key2)
    {
        locate_vplusplus(head, &pred1, &curr1, key1); //first look for key1
        if ((!curr1->v_next.load()) || curr1->data != key1)
            return false; // key1 is not present in the vertex-list

        locate_vplusplus(curr1, &pred2, &curr2, key2); // looking for key2 only if key1 present
        if ((!curr2->v_next.load()) || curr2->data != key2)
            return false; // key2 is not present in the vertex-list
    }
    else
    {
        locate_vplusplus(head, &pred2, &curr2, key2); //first look for key2
        if ((!curr2->v_next.load()) || curr2->data != key2)
            return false; // key2 is not present in the vertex-list

        locate_vplusplus(curr2, &pred1, &curr1, key1); // looking for key1 only if key2 present
        if ((!curr1->v_next.load()) || curr1->data != key1)
            return false; // key1 is not present in the vertex-list
    }
    (*n1) = curr1;
    (*n2) = curr2;
    return true;
}

void wait_free_graph ::locate_eplus(Node_List *startE, Node_List **n1, Node_List **n2, int key)
{
    Node_List *succe, *curre, *prede;
    Node_List *tv;

retry:
    while (true)
    {
        prede = startE;
        curre = prede->e_next.load();
        while (true)
        {
            succe = curre->e_next.load();
            tv = curre->point_v.load();
            /*helping: delete one or more enodes whose vertex was marked*/
        retry2:
            while ((tv != NULL && tv->v_next.load()) && curre->e_next.load() != NULL && is_marked_ref((long)tv->v_next.load()) && !is_marked_ref((long)succe) && curre->data < key)
            {
                if (!curre->e_next.compare_exchange_strong(succe, (Node_List *)get_marked_ref((long)succe), memory_order_seq_cst))
                    goto retry;
                if (!prede->e_next.compare_exchange_strong(curre, succe, memory_order_seq_cst))
                    goto retry;
                curre = (Node_List *)get_unmarked_ref((long)succe);
                succe = curre->e_next.load();
                tv = curre->point_v.load();
            }
            /*helping: delete one or more enodes which are marked*/
            while (curre->e_next.load() != NULL && is_marked_ref((long)succe) && !is_marked_ref((long)tv->v_next.load()) && curre->data < key)
            {
                if (!prede->e_next.compare_exchange_strong(curre, (Node_List *)get_unmarked_ref((long)succe), memory_order_seq_cst))
                    goto retry;
                curre = (Node_List *)get_unmarked_ref((long)succe);
                succe = curre->e_next.load();
                tv = curre->point_v.load();
            }

            if (tv && tv->v_next.load() && is_marked_ref((long)tv->v_next.load()) && curre->e_next.load() != NULL && curre->data < key)
                goto retry2;
            if (curre->data >= key)
            {
                (*n1) = prede;
                (*n2) = curre;
                return;
            }
            prede = curre;
            curre = (Node_List *)get_unmarked_ref((long)succe);
        }
    }
}

Node_List *wait_free_graph ::create_vertex(int key)
{

    Node_List *edge_head = new Node_List;
    Node_List *edge_tail = new Node_List;

    edge_head->data = INT_MIN;
    edge_head->v_next = NULL;
    edge_head->e_next = edge_tail;
    edge_head->mark = false;

    edge_tail->data = INT_MAX;
    edge_tail->v_next = NULL;
    edge_tail->e_next = NULL;
    edge_tail->mark = false;

    Node_List *vertex_node = new Node_List;

    vertex_node->data = key;
    vertex_node->v_next = NULL;
    vertex_node->e_next = edge_head;
    vertex_node->mark = false;

    return vertex_node;
}

Node_List *wait_free_graph ::create_edge(int key)
{
    Node_List *edge_node = new Node_List;

    edge_node->data = key;
    edge_node->v_next = NULL;
    edge_node->e_next = NULL;
    edge_node->mark = false;

    return edge_node;
}

bool wait_free_graph ::fast_insert_vertex(int id, int key)
{
    int tries = 0;
    while (tries++ < MAX_FAILURES)
    {
        Node_List *predv, *currv;
        locate_vplus(head, &predv, &currv, key); // find the location, <pred, curr>
        if (currv->data == key)
        {
            return false; // key already present
        }
        else
        {
            Node_List *newv = create_vertex(key); // create a new vertex node
            newv->v_next.store(currv);
            if (predv->v_next.compare_exchange_strong(currv, newv, memory_order_seq_cst))
            { // adds in the list
                return true;
            }
        }
    }
    bool result = insert_node(id, key, -1, true);
    return result;
}

bool wait_free_graph ::fast_insert_edge(int id, int key, int dest)
{
    int tries = 0;
    while (tries++ < MAX_FAILURES)
    {
        Node_List *prede, *curre;
        Node_List *u, *v;
        bool flag = contains_vplus(&u, &v, key, dest);
        if (flag == false)
        {
            return false; // either of the vertex is not present
        }
        while (true)
        {
            if (is_marked_ref((long)u->v_next.load()) || is_marked_ref((long)v->v_next.load()))
                return false;

            locate_eplus(u->e_next.load(), &prede, &curre, dest);
            if (curre->data == dest)
            {
                return false; // edge already present
            }

            Node_List *newe = create_vertex(dest); // create a new edge node
            newe->e_next.store(curre);             // connect newe->next to curr
            newe->point_v.store(v);                // points to its vertex
            if (prede->e_next.compare_exchange_strong(curre, newe, memory_order_seq_cst))
            { // actual insertion
                return true;
            }
        } // End of while
    }
    bool result = insert_node(id, key, dest, false);
    return result;
}

bool wait_free_graph ::fast_delete_vertex(int id, int key)
{
    int tries = 0;
    bool snip;
    while (tries++ < MAX_FAILURES)
    {
        Node_List *predv, *currv, *succv;
        locate_vplus(head, &predv, &currv, key);
        if (currv->data != key)
            return false; // key is not present
        succv = currv->v_next.load();
        if (!is_marked_ref((long)succv))
        {
            if (!currv->v_next.compare_exchange_strong(succv, (Node_List *)get_marked_ref((long)succv), memory_order_seq_cst))
                continue;
            if (predv->v_next.compare_exchange_strong(currv, succv, memory_order_seq_cst))
            {
                break;
            }
        }
        return true;
    }
    bool result = delete_node(id, key, -1, true);

    return result;
}

bool wait_free_graph ::fast_delete_edge(int id, int key, int dest)
{
    int tries = 0;
    bool snip;
    while (tries++ < MAX_FAILURES)
    {
        Node_List *prede, *curre, *succe;
        Node_List *u, *v;
        bool flag = contains_vplus(&u, &v, key, dest);
        if (flag == false)
        {
            return false; // either of the vertex is not present
        }

        if (is_marked_ref((long)u->v_next.load()) || is_marked_ref((long)v->v_next.load()))
            return false;
        locate_eplus(u->e_next.load(), &prede, &curre, dest);
        if (curre->data != dest)
        {
            return false; // edge already present
        }
        succe = curre->e_next.load();
        if (!is_marked_ref((long)succe))
        {
            if (!curre->e_next.compare_exchange_strong(succe, (Node_List *)get_marked_ref((long)succe), memory_order_seq_cst))
                continue;
            if (!prede->e_next.compare_exchange_strong(curre, succe, memory_order_seq_cst))
            {
                break;
            }
        }
    }
    bool result = delete_node(id, key, dest, false);

    return result;
}

bool wait_free_graph ::fast_contains_vertex(int id, int key)
{
    long int steps = 0;
    Node_List *curr = head;
    while (curr->data < key)
    {
        curr = curr->v_next.load();
        if (steps++ >= MAX_FAILURES)
        {
            return contain_node(id, key, -1, true);
        }
    }
    return (curr->data == key) && !(is_marked_ref((long)curr->v_next.load()));
}

bool wait_free_graph ::fast_contains_edge(int id, int key1, int key2)
{
    long int steps = 0;
    int tries = 0;
    while (tries++ < MAX_FAILURES)
    {
        Node_List *curr = head;
        Node_List *curre, *prede;
        Node_List *u, *v;
        bool flag = contain_cplus(&u, &v, key1, key2);
        if (flag == false)
        {
            return false; // either of the vertex is not present
        }
        curre = u->e_next.load();
        while (curre->e_next.load() && curre->data < key2)
        {
            curre = (Node_List *)get_unmarked_ref((long)curre->e_next.load());
        }
        if ((curre->e_next.load()) && curre->data == key2 && !is_marked_ref((long)curre->e_next.load()) && !is_marked_ref((long)u->v_next.load()) && !is_marked_ref((long)v->v_next.load()))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return contain_node(id, key1, key2, false);
}

bool wait_free_graph ::insert_node(int id, int key, int dest, bool is_vertex)
{
    long int phase = next_phase();
    Node_List *node;
    Node_List *dnode;
    OpDesc *op;
    if (is_vertex)
    {
        node = create_vertex(key);
        op = init_op(phase, insert_vertex, node, NULL, NULL, NULL);
        state[id].store(op, std::memory_order_seq_cst);
        help_insert_vertex(id, phase);
    }
    else
    {
        node = create_vertex(key);
        dnode = create_edge(dest);
        Node_List *u, *v;
        bool flag = contains_vplus(&u, &v, key, dest);
        if (flag == false || is_marked_ref((long int)u) || is_marked_ref((long int)v))
        {
            return false;
        }
        op = init_op(phase, insert_edge, node, dnode, u, v);
        state[id].store(op, std::memory_order_seq_cst);
        help_insert_edge(id, phase);
    }
    return (state[id].load()->type == success);
}

bool wait_free_graph ::delete_node(int id, int key, int dest, bool is_vertex)
{
    long int phase = next_phase();

    Node_List *node = new Node_List;
    Node_List *dnode = new Node_List;

    if (is_vertex)
    {
        node->data = key;
        node->mark = false;
        OpDesc *op = init_op(phase, search_delete_vertex, node, NULL, NULL, NULL);
        state[id].store(op, std::memory_order_seq_cst);
        help_delete_vertex(id, phase);
        op = state[id].load();
        if (op->type == success)
        {
            return true;
        }
        return false;
    }
    else
    {
        node->data = key;
        node->mark = false;
        dnode->data = dest;
        dnode->mark = false;
        OpDesc *op;
        Node_List *u, *v;
        bool flag = contains_vplus(&u, &v, key, dest);
        if (flag == false || is_marked_ref((long int)u) || is_marked_ref((long int)v))
        {
            return false;
        }
        op = init_op(phase, insert_edge, node, dnode, u, v);
        state[id].store(op, std::memory_order_seq_cst);
        help_delete_edge(id, phase);
        op = state[id].load();
        if (op->type == success)
        {
            return true;
        }
        return false;
    }
}

bool wait_free_graph ::contain_node(int id, int key, int dest, bool is_vertex)
{
    long int phase = next_phase();
    Node_List *node;
    Node_List *dnode;
    OpDesc *op;
    if (is_vertex)
    {
        node = create_vertex(key);
        op = init_op(phase, contains_vertex, node, NULL, NULL, NULL);
        state[id].store(op, std::memory_order_seq_cst);
        help_contains_vertex(id, phase);
    }
    else
    {
        node = create_vertex(key);
        dnode = create_edge(dest);
        Node_List *u, *v;
        bool flag = contains_vplus(&u, &v, key, dest);
        if (flag == false || is_marked_ref((long int)u) || is_marked_ref((long int)v))
        {
            return false;
        }
        op = init_op(phase, insert_edge, node, dnode, u, v);
        state[id].store(op, std::memory_order_seq_cst);
        help_contains_edge(id, phase);
    }
    return (state[id].load()->type == success);
}

void wait_free_graph ::help_insert_vertex(int id, long int phase)
{
    while (true)
    {
        OpDesc *op = state[id].load();
        if (!(op->type == insert_vertex && op->phase == phase))
        {
            return;
        }
        Node_List *node = op->node;
        Node_List *next_node = node->v_next;
        Node_List *prev, *curr;
        locate_vplus(head, &prev, &curr, node->data);
        if (curr->data == node->data)
        {
            if (curr == node || is_marked_ref((long)next_node))
            {
                OpDesc *suc = init_op(phase, success, node, NULL, NULL, NULL);
                if (atomic_compare_exchange_strong_explicit(&state[id], &op, suc, std::memory_order_seq_cst, std::memory_order_seq_cst))
                {
                    return;
                }
            }
            else
            {
                OpDesc *fail = init_op(phase, failure, node, NULL, NULL, NULL);
                if (atomic_compare_exchange_strong_explicit(&state[id], &op, fail, std::memory_order_seq_cst, std::memory_order_seq_cst))
                {
                    return;
                }
            }
        }
        else
        {
            if (is_marked_ref((long)next_node))
            {
                OpDesc *suc = init_op(phase, success, node, NULL, NULL, NULL);
                if (atomic_compare_exchange_strong_explicit(&state[id], &op, suc, std::memory_order_seq_cst, std::memory_order_seq_cst))
                {
                    return;
                }
            }
            OpDesc *newOp = init_op(phase, insert_vertex, node, NULL, NULL, NULL);
            if (!atomic_compare_exchange_strong_explicit(&state[id], &op, newOp, std::memory_order_seq_cst, std::memory_order_seq_cst))
            {
                continue;
            }
            atomic_compare_exchange_strong_explicit(&node->v_next, &next_node, curr, std::memory_order_seq_cst, std::memory_order_seq_cst);

            if (atomic_compare_exchange_strong_explicit(&prev->v_next, &curr, node, std::memory_order_seq_cst, std::memory_order_seq_cst))
            {
                OpDesc *suc = init_op(phase, success, node, NULL, NULL, NULL);
                if (atomic_compare_exchange_strong_explicit(&state[id], &newOp, suc, std::memory_order_seq_cst, std::memory_order_seq_cst))
                {
                    return;
                }
            }
        }
    }
}

void wait_free_graph ::help_delete_vertex(int id, long int phase)
{
    while (true)
    {
        OpDesc *op = state[id].load();
        if (!((op->type == search_delete_vertex) && op->phase == phase))
        {
            return;
        }
        Node_List *node = op->node;
        Node_List *prev, *curr;
        locate_vplus(head, &prev, &curr, node->data);
        Node_List *next_node = curr->v_next;
        if (curr == NULL && prev == NULL)
        {
            return;
        }
        if (curr->data != node->data)
        {
            OpDesc *fail = init_op(phase, failure, node, NULL, NULL, NULL);
            if (atomic_compare_exchange_strong_explicit(&state[id], &op, fail, std::memory_order_seq_cst, std::memory_order_seq_cst))
            {
                return;
            }
        }
        Node_List *marked_next_node;
        if (!is_marked_ref((long)next_node))
        {
            marked_next_node = (Node_List *)get_marked_ref((long)next_node);
            if (!atomic_compare_exchange_strong_explicit(&curr->v_next, &next_node, marked_next_node, memory_order_seq_cst, memory_order_seq_cst))
            {
                continue;
            }
        }
        if (!atomic_compare_exchange_strong_explicit(&prev->v_next, &curr, next_node, memory_order_seq_cst, memory_order_seq_cst))
        {
            continue;
        }
        OpDesc *suc = init_op(phase, success, node, NULL, NULL, NULL);
        if (atomic_compare_exchange_strong_explicit(&state[id], &op, suc, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            return;
        }
    }
}

void wait_free_graph ::help_contains_vertex(int id, long int phase)
{
    OpDesc *op = state[id].load();
    if (!(op->type == contains_vertex && op->phase == phase))
    {
        return;
    }
    Node_List *node = op->node;
    Node_List *prev, *curr;
    locate_vplus(head, &prev, &curr, node->data);
    Node_List *next_node = curr->e_next;
    if (prev == NULL || curr == NULL)
    {
        return;
    }
    if (curr->data == node->data && !is_marked_ref((long)(curr->v_next.load())))
    {
        OpDesc *suc = init_op(phase, success, op->node, NULL, prev, curr);
        atomic_compare_exchange_strong_explicit(&state[id], &op, suc, std::memory_order_seq_cst, std::memory_order_seq_cst);
    }
    else
    {
        OpDesc *fail = init_op(phase, failure, op->node, NULL, NULL, NULL);
        atomic_compare_exchange_strong_explicit(&state[id], &op, fail, std::memory_order_seq_cst, std::memory_order_seq_cst);
    }
}

void wait_free_graph ::help_insert_edge(int id, long int phase)
{
    while (true)
    {
        OpDesc *op = state[id].load();
        if (!(op->type == insert_edge && op->phase == phase))
        {
            return;
        }
        Node_List *node = op->node;
        Node_List *dnode = op->dnode;
        Node_List *next_node = dnode->e_next;
        Node_List *ptv = dnode->point_v;
        Node_List *prev_v, *curr_v;
        if (is_marked_ref((long)(op->prev->v_next.load()) || is_marked_ref((long)op->curr->v_next.load())))
            return;
        Node_List *prev, *curr;
        locate_eplus(op->prev->e_next.load(), &prev, &curr, dnode->data);

        if (prev == NULL && curr == NULL)
        {
            return;
        }

        if (curr->data == dnode->data)
        {
            if (curr == dnode || is_marked_ref((long)next_node))
            {
                OpDesc *suc = init_op(phase, success, node, dnode, NULL, NULL);
                if (atomic_compare_exchange_strong_explicit(&state[id], &op, suc, std::memory_order_seq_cst, std::memory_order_seq_cst))
                {
                    return;
                }
            }
            else
            {
                OpDesc *fail = init_op(phase, failure, node, dnode, NULL, NULL);
                if (atomic_compare_exchange_strong_explicit(&state[id], &op, fail, std::memory_order_seq_cst, std::memory_order_seq_cst))
                {
                    return;
                }
            }
        }
        else
        {
            if (is_marked_ref((long)next_node))
            {
                OpDesc *suc = init_op(phase, success, node, dnode, NULL, NULL);
                if (atomic_compare_exchange_strong_explicit(&state[id], &op, suc, std::memory_order_seq_cst, std::memory_order_seq_cst))
                {
                    return;
                }
            }
            OpDesc *newOp = init_op(phase, insert_edge, node, dnode, NULL, NULL);
            if (!atomic_compare_exchange_strong_explicit(&state[id], &op, newOp, std::memory_order_seq_cst, std::memory_order_seq_cst))
            {
                continue;
            }
            atomic_compare_exchange_strong_explicit(&dnode->e_next, &next_node, curr, std::memory_order_seq_cst, std::memory_order_seq_cst);

            if (atomic_compare_exchange_strong_explicit(&prev->e_next, &curr, dnode, std::memory_order_seq_cst, std::memory_order_seq_cst))
            {
                dnode->point_v = op->curr;
                OpDesc *suc = init_op(phase, success, node, dnode, NULL, NULL);
                if (atomic_compare_exchange_strong_explicit(&state[id], &newOp, suc, std::memory_order_seq_cst, std::memory_order_seq_cst))
                {
                    Node_List *temp = prev->e_next;
                    return;
                }
            }
        }
    }
}

void wait_free_graph ::help_delete_edge(int id, long int phase)
{
    while (true)
    {
        OpDesc *op = state[id].load();
        if (!((op->type == search_delete_edge) && op->phase == phase))
        {
            return;
        }
        if (is_marked_ref((long)(op->prev->v_next.load()) || is_marked_ref((long)op->curr->v_next.load())))
        {
            OpDesc *fail = init_op(phase, failure, op->node, NULL, NULL, NULL);
            atomic_compare_exchange_strong_explicit(&state[id], &op, fail, std::memory_order_seq_cst, std::memory_order_seq_cst);
            return;
        }
        Node_List *node = op->node;
        Node_List *dnode = op->dnode;
        Node_List *prev, *curr;
        locate_eplus(op->prev->e_next.load(), &prev, &curr, dnode->data);
        Node_List *next_node = curr->e_next;
        if (curr == NULL && prev == NULL)
        {
            return;
        }
        if (curr->data != dnode->data)
        {
            OpDesc *fail = init_op(phase, failure, node, dnode, NULL, NULL);
            if (atomic_compare_exchange_strong_explicit(&state[id], &op, fail, std::memory_order_seq_cst, std::memory_order_seq_cst))
            {
                return;
            }
        }
        Node_List *marked_next_node;
        if (!is_marked_ref((long)next_node))
        {
            marked_next_node = (Node_List *)get_marked_ref((long)next_node);
            if (!atomic_compare_exchange_strong_explicit(&curr->e_next, &next_node, marked_next_node, memory_order_seq_cst, memory_order_seq_cst))
            {
                continue;
            }
        }
        if (!atomic_compare_exchange_strong_explicit(&prev->e_next, &curr, next_node, memory_order_seq_cst, memory_order_seq_cst))
        {
            continue;
        }
        OpDesc *suc = init_op(phase, success, node, dnode, NULL, NULL);
        if (atomic_compare_exchange_strong_explicit(&state[id], &op, suc, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            return;
        }
    }
}

void wait_free_graph ::help_contains_edge(int id, long int phase)
{
    OpDesc *op = state[id].load();
    if (!(op->type == contains_edge && op->phase == phase))
    {
        return;
    }
    if (is_marked_ref((long)(op->prev->v_next.load()) || is_marked_ref((long)op->curr->v_next.load())))
    {
        OpDesc *fail = init_op(phase, failure, op->node, NULL, NULL, NULL);
        atomic_compare_exchange_strong_explicit(&state[id], &op, fail, std::memory_order_seq_cst, std::memory_order_seq_cst);
        return;
    }
    Node_List *node = op->node;
    Node_List *dnode = op->dnode;
    Node_List *prev, *curr;
    locate_eplus(op->prev->e_next.load(), &prev, &curr, dnode->data);
    if (curr->data == dnode->data && !is_marked_ref((long)(curr->e_next.load())))
    {
        OpDesc *suc = init_op(phase, success, node, dnode, NULL, NULL);
        atomic_compare_exchange_strong_explicit(&state[id], &op, suc, std::memory_order_seq_cst, std::memory_order_seq_cst);
        return;
    }
    else
    {
        OpDesc *fail = init_op(phase, failure, node, dnode, NULL, NULL);
        atomic_compare_exchange_strong_explicit(&state[id], &op, fail, std::memory_order_seq_cst, std::memory_order_seq_cst);
        return;
    }
}

long int wait_free_graph ::next_phase()
{
    return max_phase.fetch_add(1, std::memory_order_seq_cst);
}

atomic<long> vertexID;
double seconds;
struct timeval tv1, tv2;
TIME_DIFF *difference;
int NTHREADS, ops;

typedef struct infothread
{
    long tid;
    wait_free_graph *G;
} tinfo;

void *pthread_call(void *t)
{
    tinfo *ti = (tinfo *)t;
    long Tid = ti->tid;
    wait_free_graph *G1 = ti->G;
    int u, v;
    int other, res;

    long long int numOfOperations = 10000000000;
    long long int numOfOperations_addEdge = numOfOperations * 0.025;       // 25% for add edge
    long long int numOfOperations_addVertex = numOfOperations * 0.025;     // 25% for add vertex
    long long int numOfOperations_removeVertex = numOfOperations * 0.025;  // 10% for remove vertex
    long long int numOfOperations_removeEdge = numOfOperations * 0.025;    // 10% for remove edge
    long long int numOfOperations_containsVertex = numOfOperations * 0.45; // 15% for contains vertex
    long long int numOfOperations_containsEdge = numOfOperations * 0.45;   // 15% for contains edge

    long long int total = numOfOperations_addEdge + numOfOperations_addVertex + numOfOperations_removeVertex + numOfOperations_removeEdge; // + numOfOperations_containsVertex + numOfOperations_containsEdge;

    while (total > 0)
    {
        gettimeofday(&tv2, NULL);
        difference = my_difftime(&tv1, &tv2);

        if (difference->secs >= seconds)
            break;

        int other = rand() % 6;
        if (other == 0)
        {
            if (numOfOperations_addEdge > 0)
            {
            l1:
                u = (rand() % (vertexID)); //vertex IDs are from 1
                v = (rand() % (vertexID));
                if (u == v || u == 0 || v == 0) //simple graph without self loops
                    goto l1;
                res = G1->fast_insert_edge((int)Tid, u, v);
                numOfOperations_addEdge = numOfOperations_addEdge - 1;
                total = total - 1;
                ops++;
            }
        }
        else if (other == 1)
        {
            if (numOfOperations_addVertex > 0)
            {
                v = vertexID; //vertices do not come again
                vertexID++;
                res = G1->fast_insert_vertex((int)Tid, v);
                numOfOperations_addVertex = numOfOperations_addVertex - 1;
                ops++;
                total = total - 1;
            }
        }
        else if (other == 2)
        {
            if (numOfOperations_removeVertex > 0)
            {
            l2:
                v = rand() % (vertexID); //dont decrement the total vertex count
                if (v == 0)
                    goto l2;
                res = G1->fast_delete_vertex((int)Tid, v);
                numOfOperations_removeVertex = numOfOperations_removeVertex - 1;
                ops++;
                total = total - 1;
            }
        }
        else if (other == 3)
        {
            if (numOfOperations_removeEdge > 0)
            {
            l3:
                u = (rand() % (vertexID)); //vertex IDs are from 1
                v = (rand() % (vertexID));
                if (u == v || u == 0 || v == 0)
                    goto l3;
                res = G1->fast_delete_edge((int)Tid, u, v);
                numOfOperations_removeEdge = numOfOperations_removeEdge - 1;
                ops++;
                total = total - 1;
            }
        }
        else if (other == 4)
        {
            if (numOfOperations_containsVertex > 0)
            {
            l4:
                u = (rand() % (vertexID)); //vertex IDs are from 1
                v = (rand() % (vertexID));
                if (u == v || u == 0 || v == 0)
                    goto l4;
                res = G1->contain_node((int)Tid, u, v, true);
                numOfOperations_containsVertex = numOfOperations_containsVertex - 1;
                ops++;
                total = total - 1;
            }
        }
        else if (other == 5)
        {
            if (numOfOperations_containsEdge > 0)
            {
            l5:
                u = (rand() % (vertexID)); //vertex IDs are from 1
                v = (rand() % (vertexID));
                if (u == v || u == 0 || v == 0)
                    goto l5;
                res = G1->contain_node((int)Tid, u, v, false);
                numOfOperations_containsEdge = numOfOperations_containsEdge - 1;
                ops++;
                total = total - 1;
            }
        }
    } //end of while loop
}

int main(int argc, char *argv[]) //command line arguments - #threads, #vertices initially, #time in seconds
{
    vertexID.store(1);
    int i;

    if (argc < 3)
    {
        cout << "Enter 3 command line arguments - #threads, #vertices initially, #time in seconds" << endl;
        return 0;
    }

    NTHREADS = atoi(argv[1]);
    int initial_vertices = atoi(argv[2]); // initial number of vertices
    seconds = atoi(argv[3]);
    ops = 0;

    vertexID.store(initial_vertices + 1); // or +1?

    wait_free_graph *sg = new wait_free_graph(NTHREADS);

    for (int i = 1; i <= initial_vertices; i++)
    {
        sg->insert_node(0, i, -1, true);
    }

    cout << "Number of Threads: " << NTHREADS << endl;
    cout << "Initial graph with " << initial_vertices << " created." << endl;
    pthread_t *thr = new pthread_t[NTHREADS];
    // Make threads Joinable for sure.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int dig, temp;
    double duration = 0.0;

    gettimeofday(&tv1, NULL);
    cout << "timer started . . ." << endl;

    for (i = 0; i < NTHREADS; i++)
    {
        tinfo *t = (tinfo *)malloc(sizeof(tinfo));
        t->tid = i;
        t->G = sg;
        pthread_create(&thr[i], &attr, pthread_call, (void *)t);
    }

    for (i = 0; i < NTHREADS; i++)
    {
        pthread_join(thr[i], NULL);
    }

    cout << seconds << " seconds elapsed" << endl;

    cout << "Total operations: " << ops << endl;

    return 0;
}
