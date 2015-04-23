#include <realm.hpp>
#include <realm/group_shared.hpp>
#include <pthread.h>

#include <unistd.h>

using namespace realm;

struct thread_info {
    pthread_t thread_id;
    int       thread_num;
};


REALM_TABLE_3(People,
                name, String,
                age,  Int,
                hired, Bool)

REALM_TABLE_2(Books,
                title, String,
                author, String)


void* reader(void*)
{
    SharedGroup sg("test.realm");

    // Read transaction
    {
        const Group& g = sg.begin_read();
        Books::ConstRef t = g.get_table<Books>("books");
        std::cout << "Books: " << t->size() << std::endl;
        sg.end_read();
    }

    while (!sg.has_changed()) { // wait for an update
        sleep(2);
        std::cout << "No updates" << std::endl;
    }

    {
        const Group& g = sg.begin_read();
        Books::ConstRef t = g.get_table<Books>("books");
        std::cout << "Books: " << t->size() << std::endl;
        sg.end_read();
    }

    return NULL;
}

void* writer(void*)
{
    SharedGroup sg("test.realm");

    sleep(5);

    // Write transaction
    {
        std::cout << "Adding book" << std::endl;
        Group& g = sg.begin_write();
        Books::Ref t = g.get_table<Books>("books");
        t->add("Solaris", "Stanislaw Lem");
        sg.commit();
    }

    return NULL;
}

int main()
{
    pthread_attr_t attr;
    struct thread_info tinfo[2];
    void *res;

    pthread_attr_init(&attr);
    tinfo[0].thread_num = 1;
    tinfo[1].thread_num = 2;

    pthread_create(&tinfo[0].thread_id, &attr, &reader, &tinfo[0]);
    pthread_create(&tinfo[1].thread_id, &attr, &writer, &tinfo[1]);

    pthread_join(tinfo[0].thread_id, &res);
    pthread_join(tinfo[1].thread_id, &res);
}
