#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

sem_t reader_mutex, writer_mutex, write_lock, read_try_lock, output_lock, reader_count_lock;
sem_t reader_checker, writer_checker;
int read_counter = 0, write_counter = 0;

void read_num_readers(FILE *otpt){
    if (otpt){
        fprintf(otpt, "Reading,Number-of-readers-present:[%d]", read_counter);
        fprintf(otpt, "\n");
        fclose(otpt);
    }
}

void *reader(void *arg) {
    sem_wait(&reader_mutex);
    sem_wait(&read_try_lock);
    sem_wait(&reader_count_lock);
    read_counter++;
    if (read_counter == 1){
        sem_post(&reader_count_lock);
        sem_wait(&write_lock);
    }
    else sem_post(&reader_count_lock);
    sem_post(&read_try_lock);
    sem_post(&reader_mutex);
    sem_wait(&output_lock);
    FILE *otpt = fopen("output-writer-pref.txt", "a");
    if (otpt) read_num_readers(otpt);
    sem_post(&output_lock);
    FILE *shared_file = fopen("shared-file.txt", "r");
    if (shared_file){
        char buffer[10000];
        int x;
        while (fgets(buffer, sizeof(buffer), shared_file) != NULL) x = 1;
        fclose(shared_file);
    }
    sem_wait(&reader_count_lock);
    read_counter--;
    if (read_counter == 0) sem_post(&write_lock);
    sem_post(&reader_count_lock);
    return NULL;
}

void write_hello_world(FILE *shared_file){
    if (shared_file) {  
        fprintf(shared_file, "Hello world!");
        fprintf(shared_file, "\n");
        fflush(shared_file); 
        fclose(shared_file); 
    }
}

void write_num_readers(FILE *otpt){
    if (otpt){
        fprintf(otpt, "Writing,Number-of-readers-present:[%d]", read_counter);
        fprintf(otpt, "\n");
        fclose(otpt);
    }
}

void *writer(void *arg){
    sem_wait(&writer_mutex);
    write_counter++;
    if (write_counter == 1) sem_wait(&read_try_lock);
    sem_post(&writer_mutex);
    sem_wait(&write_lock);

    sem_wait(&output_lock);
    FILE *otpt = fopen("output-writer-pref.txt", "a");
    if (otpt) write_num_readers(otpt);
    sem_post(&output_lock);
    FILE *shared_file = fopen("shared-file.txt", "a");
    if (shared_file) write_hello_world(shared_file);
    sem_post(&write_lock);
    sem_wait(&writer_mutex);
    write_counter--;
    if (write_counter == 0) sem_post(&read_try_lock);
    sem_post(&writer_mutex);
    return NULL;
}

int check_num_args(int argc){
    if (argc != 3) return 0;
    return 1;
}

int main(int argc, char **argv){
    int check_args = check_num_args(argc);
    if (!check_args) return 1;

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    pthread_t readers[n], writers[m];

    sem_init(&reader_mutex, 0, 1);
    sem_init(&writer_mutex, 0, 1);
    sem_init(&write_lock, 0, 1);
    sem_init(&read_try_lock, 0, 1);
    sem_init(&output_lock, 0, 1);
    sem_init(&reader_count_lock, 0, 1);

    int i = 0;
    while (i < n){
        pthread_create(&readers[i], NULL, reader, NULL);
        i++;
    }
    i = 0;
    while (i < m){
        pthread_create(&writers[i], NULL, writer, NULL);
        i++;
    }
    i = 0;
    while (i < n){
        pthread_join(readers[i], NULL);
        i++;
    }
    i = 0;
    while (i < m){
        pthread_join(writers[i], NULL);
        i++;
    }

    // Destroy semaphores
    sem_destroy(&reader_mutex);
    sem_destroy(&writer_mutex);
    sem_destroy(&write_lock);
    sem_destroy(&read_try_lock);
    sem_destroy(&output_lock);
    sem_destroy(&reader_count_lock);
    sem_destroy(&reader_checker);
    sem_destroy(&writer_checker);
    return 0;
}
