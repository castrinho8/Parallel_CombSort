/*
 * Práctica Final AEC. FIC. UDC. 2012/2013
 *   Pablo Castro Valiño (pablo.castro1@udc.es)
 *   Marcos Chavarría Teijeiro (marcos.chavarria@udc.es)
 */

#include <stdlib.h>
#include <assert.h>

struct heap
{
	int 	size;
	int 	count;
	int 	*data;
	short 	*proc;
};

typedef struct heap *heap_t;


heap_t
new_heap(int ini_size)
{
	heap_t h;

	h = malloc(sizeof(struct heap));
	h->count = 0;
	h->size = ini_size;
	h->data = malloc(sizeof(int) * ini_size);
	h->proc = malloc(sizeof(short) * ini_size);

	return h;
}

int
_get_first_child(int pos)
{
	return (pos * 2) + 1;
}

int
_get_father(int pos)
{
	if (pos) return (pos-1)/2;
	return -1;
}

void
_swap(heap_t h, int i, int j)
{
	int tmpvalue;
	short tmpproc;

	tmpvalue = h->data[i];
	h->data[i] = h->data[j];
	h->data[j] = tmpvalue;

	tmpproc = h->proc[i];
	h->proc[i] = h->proc[j];
	h->proc[j] = tmpproc;
}

int
empty_heap(heap_t h)
{
	return h && h->count == 0;
}

void
push_heap	(heap_t h,
			int value,
			int proc)
{
	int current, father;
	if (h->count == h->size)
	{
		h->size <<= 1;
		h->data = realloc(h->data, sizeof(int) * h->size);
		h->proc = realloc(h->proc, sizeof(int) * h->size);
		assert(h->data);
		assert(h->proc);
	}

	h->data[h->count] = value;
	h->proc[h->count] = proc;

	current = h->count;
	father = _get_father(current);

	while(father != -1 && h->data[father] > h->data[current])
	{
		_swap(h,current,father);
		current = father;
		father = _get_father(current);
	}

	h->count++;
}


void
pop_heap	(heap_t h,
			int *value,
			int *proc)
{
	int current, first_child;

	assert(!empty_heap(h));
	*value = h->data[0];
	*proc = h->proc[0];

	h->count--;

	h->data[0] = h->data[h->count];
	h->proc[0] = h->proc[h->count];

	current = 0;
	first_child = _get_first_child(current);

	while (	(first_child     < h->count && h->data[current] > h->data[first_child]) ||
			(first_child + 1 < h->count && h->data[current] > h->data[first_child+1]))
	{
		if(first_child + 1 >= h->count || h->data[first_child] < h->data[first_child + 1])
		{
			_swap(h,current,first_child);
			current = first_child;
			first_child = _get_first_child(current);
		}
		else
		{
			_swap(h,current,first_child+1);
			current = first_child+1;
			first_child = _get_first_child(current);
		}
	}
}

void
dispose_heap(heap_t h)
{
	h->count = 0;
	h->size = 0;
	free(h->data);
	free(h->proc);
	h->data = NULL;
	h->proc = NULL;
}

