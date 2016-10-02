/*
===========================================================

Name    :   oed_mem.h

Purpose :   memory controller class

Modified:   11/03/2006

===========================================================
*/

#ifndef __OED_MEM__
#define __OED_MEM__

#include "oed_shared.h"
#include "oed_error.h"

#include <stdlib.h> // malloc free srand rand

#define TAG_FREE    0
#define LINK_TO_PTR(a)  ((void *)(a+1))
#define PTR_TO_LINK(a)  (((mem_link_s *)a)-1)

class memctrl_c
{
private:
    unsigned int    mem_bytes;
    unsigned short  mem_count;
    unsigned short  mem_id;

    struct mem_link_s
    {
        mem_link_s  *prev, *next;
        unsigned int        size;
        unsigned short      id;
        unsigned short      tag;
    };

    class memerror_c : public errorobj_c
    {
    private:
        memerror_c () {}
    public:
        memerror_c (char *message, bool fatal) : errorobj_c(message,fatal) { strcpy( m_type, "memory" ); }
        memerror_c (char *message, int code, bool fatal) : errorobj_c(message,code,fatal) { strcpy( m_type, "memory" ); }
    };

    mem_link_s  mem_head;

    int     link_size;

public:
    memctrl_c () : link_size(sizeof(mem_link_s))    // random id
    {
        mem_head.next = mem_head.prev = &mem_head;
#pragma warning (disable:4311)
        srand((unsigned )(uintptr_t)this);
#pragma warning (default:4311)
        mem_id = (rand()&(0xffff));
    }
    ~memctrl_c () { clear(); }

    void init ()
    {
        mem_head.next = mem_head.prev = &mem_head;
#pragma warning (disable:4311)
        srand((unsigned )(uintptr_t)this);
#pragma warning (default:4311)
        mem_id = (rand()&(0xffff));
    }


    void query (int &blocks, int &bytes)
    {
        blocks = mem_count;
        bytes = mem_bytes;
    }

    void *alloc (unsigned int size) { return alloct(size,TAG_FREE); }   // allocate
    void *alloct (unsigned int size, unsigned short tag)    // allocate with a tag
    {
        mem_link_s  *link;

        size = size + link_size;
        link = (mem_link_s *)malloc( size );

        if ( !link )
            return NULL;

        link->size = size;
        link->id = mem_id;              // internal id
        link->tag = tag;

        link->next = &mem_head;         // end of chain
        link->prev = mem_head.prev;
        link->next->prev = link;
        link->prev->next = link;

        mem_count++;
        mem_bytes+=size;

        return LINK_TO_PTR(link);
    }

    void m_free (void *ptr) // free ptr
    {
        mem_link_s  *link;

        link = PTR_TO_LINK(ptr);
        if (link->id != mem_id) // bad id
            throw( memerror_c( "mem::free | bad pointer\n", ERROR_BADPTR, true ) );

        link->next->prev = link->prev;
        link->prev->next = link->next;

        mem_count--;
        mem_bytes-=link->size;

        free (link);
    }

    void freet (unsigned short tag) // frees all of given tag
    {
        mem_link_s *link, *next;
        for (link = mem_head.next,next=NULL;link!=&mem_head;link = next)
        {
            next = link->next;
            if (link->tag == tag)
                m_free(LINK_TO_PTR(link));
        }
    }

    void clear()    // frees everything
    {
        mem_link_s *link, *next;
        for (link = mem_head.next,next=NULL;link!=&mem_head;link = next)
        {
            next = link->next;
            m_free(LINK_TO_PTR(link));
        }
    }

};

extern memctrl_c    *s_memctrl_c;

namespace mem
{
    inline void init ()                                     { s_memctrl_c->init( ); }
    inline void query (int &blocks, int &bytes)             { s_memctrl_c->query( blocks, bytes ); }
    inline void *alloc (unsigned int size)                  { return s_memctrl_c->alloct(size,TAG_FREE); }  // allocate
    inline void *alloct (unsigned int size, unsigned short tag)     { return s_memctrl_c->alloct(size,tag); }   // allocate with a tag
    inline void free (void *ptr)                            { return s_memctrl_c->m_free(ptr); }    // free ptr
    inline void freet (unsigned short tag)                  { return s_memctrl_c->freet(tag); }// frees all of given tag
    inline void clear()                                     { s_memctrl_c->clear(); }   // frees everything
}

class memobj_c
{
public:
    void * operator new (size_t s) { return mem::alloc( s ); }
    void operator delete (void *ptr) { mem::free( ptr ); }
};

#endif // __OED_MEM__
