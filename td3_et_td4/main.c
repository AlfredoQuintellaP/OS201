#include <signal.h>
#include <stddef.h>
#include <stdio.h>  // getchar
#include <stdlib.h> // exit
#include <sys/mman.h>

#include <fcntl.h>
#include <unistd.h>

// Pour compiler ce fichier et le linker avec les fonctions assembleur:
// $ gcc -static -Wall tp3-skeleton.c tp3-skeleton.s -o tp3

// Vous pouvez ensuite lancer le programme compilÃ©:
// $ ./tp3

// Sinon "make" devrait suffir à créer l'executable "tp"

// Note: print_str in coroutines/threads segfaults on some machines, if that is
// the case on yours, replace the print_str's with calls to the handcoded
// print_str and print_int

// print a string char by char using putchar
void print_str(char *str) {
  while (*str) {
    putchar((int)*str);
    ++str;
  }
}

// print a integer in base 10 using putchar
void print_int(int x) {
  if (x < 0) {
    putchar('-');
    x = -x;
  }
  if (x == 0) {
    putchar('0');
    return;
  }
  int pos = 1;
  while (x >= pos) {
    pos *= 10;
  }
  while (pos > 1) {
    pos /= 10;
    putchar('0' + (x / pos) % 10);
  }
}

// The size of our stacks
#define STACK_SIZE_FULL 4096
char stack1[STACK_SIZE_FULL];
char stack2[STACK_SIZE_FULL];
char stack3[STACK_SIZE_FULL];
char stack4[STACK_SIZE_FULL];

typedef void *coroutine_t;
coroutine_t cr1, cr2;
coroutine_t current;



/* Quitte le contexte courant et charge les registres et la pile de CR. */
void enter_coroutine(coroutine_t cr);

/* Sauvegarde le contexte courant dans p_from, et entre dans TO. */
void switch_coroutine(coroutine_t *p_from, coroutine_t to);

/* Initialise la pile et renvoie une coroutine telle que, lorsqu’on entrera
dedans, elle commencera à s’exécuter à l’adresse initial_pc. */

coroutine_t init_coroutine(void *stack_begin, size_t stack_size,
                           void (*initial_pc)()) {
  char *stack_end = ((char *)stack_begin) + stack_size;
  void **ptr = (void **)stack_end;

  
  ptr--;
  *ptr = initial_pc;
  ptr--;
  *ptr = NULL; // rbp
  ptr--;
  *ptr = NULL; // rbx
  ptr--;
  *ptr = NULL; // r12
  ptr--;
  *ptr = NULL; // r13
  ptr--;
  *ptr = NULL; // r14
  ptr--;
  *ptr = NULL; // r15

  return ptr;
}

void coroutine1() {
    int i = 0;
    while (1) {
        print_str("[Coroutine 1] i = ");
        print_int(i++);
        print_str("\n");
        switch_coroutine(&cr1, cr2);  // passa controle para cr2
    }
}

void coroutine2() {
    int j = 0;
    while (1) {
        print_str("[Coroutine 2] j = ");
        print_int(j++);
        print_str("\n");
        switch_coroutine(&cr2, cr1);  // passa controle para cr1
    }
}

coroutine_t scheduler, curr_thread;

enum status { READY, BLOCKED };

struct thread {
  coroutine_t *coroutine;
  enum status status;
};

size_t nb_threads;
struct thread *threads;

void yield(void) {
    switch_coroutine(&curr_thread, scheduler);
}

struct thread thread_create(void *stack, void (*f)()) {
  struct thread th = {
    .coroutine = init_coroutine(stack, STACK_SIZE_FULL , f),
    .status = READY
  };
  return th;
}

void schedule() {
    size_t current = 0;
    while(1) {
        for(size_t i = 0; i < nb_threads; i++) {
            current = (current + 1) % nb_threads;
            if(threads[current].status == READY) {
                curr_thread = threads[current].coroutine;
                switch_coroutine(&scheduler, curr_thread);
                threads[current].coroutine = curr_thread;
                break;
            }
        }
    }
}

// Threads de teste para verificar o scheduler
void thread1() {
    int counter = 0;
    while(1) {
        print_str("Thread 1 - Counter: ");
        print_int(counter++);
        print_str("\n");
        yield();  // Passa controle para scheduler
    }
}

void thread2() {
    int value = 100;
    while(1) {
        print_str("Thread 2 - Value: ");
        print_int(value++);
        print_str("\n");
        yield();  // Passa controle para scheduler
    }
}

void thread3() {
    int x = 0;
    while(1) {
        print_str("Thread 3 - X: ");
        print_int(x += 2);
        print_str("\n");
        yield();  // Passa controle para scheduler
    }
}

#define TEST_INIT 0
#define TEST_SWITCH 1  
#define TEST_THREAD 2
#define TEST_PRODUCER 3

#define TEST TEST_THREAD  // Mude para testar diferentes partes

int main() {
  fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
  
#if TEST == TEST_THREAD
  nb_threads = 3;
  struct thread thrs[3] = {
    thread_create(stack2, &thread1),
    thread_create(stack3, &thread2), 
    thread_create(stack4, &thread3)
  };
  threads = thrs;
  scheduler = init_coroutine(stack1, STACK_SIZE_FULL, &schedule);
  enter_coroutine(scheduler);
#endif

  return 0;
}
