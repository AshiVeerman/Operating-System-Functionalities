#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

sem_t read_counter_lock, resource_access_lock, file_access_lock;
sem_t reader_checker, writer_checker;
int counter = 0;

void read_num_readers(FILE *otpt){
    if (otpt){
        fprintf(otpt, "Reading,Number-of-readers-present:[%d]", counter);
        fprintf(otpt, "\n");
        fclose(otpt);
    }
}

void *reader(void *arg)
{
    // Lock to update counter
    sem_wait(&read_counter_lock);
    counter++;
    if (counter == 1) sem_wait(&resource_access_lock); // First reader locks writers out
    sem_post(&read_counter_lock);

    // Lock to safely write output
    sem_wait(&file_access_lock);
    FILE *otpt = fopen("output-reader-pref.txt", "a");
    if (otpt) read_num_readers(otpt);
    sem_post(&file_access_lock);

    FILE *shared_file = fopen("shared-file.txt", "r");
    if (shared_file) 
    {
        char buffer[10000];
        int x;
        while (fgets(buffer, sizeof(buffer), shared_file) != NULL) { 
            x=1;
        }
        fclose(shared_file);
    }

    // Lock to update reader count after finishing read
    sem_wait(&read_counter_lock);
    counter--;
    if (counter == 0) sem_post(&resource_access_lock); // Last reader allows writers
    sem_post(&read_counter_lock);
    return NULL;
}

void write_hello_world(FILE *shared_file){
    if (shared_file) {  // Check if the file is opened successfully
        fprintf(shared_file, "Hello world!");
        fprintf(shared_file, "\n");
        fflush(shared_file); // Ensure that the data is written immediately
        fclose(shared_file); // Close the file after writing
    }
}

void write_num_readers(FILE *otpt){
    if (otpt){
        fprintf(otpt, "Writing,Number-of-readers-present:[%d]", counter);
        fprintf(otpt, "\n");
        fclose(otpt);
    }
}

void *writer(void *arg) 
{
    sem_wait(&resource_access_lock); // Lock resource for exclusive writer access
    
    sem_wait(&file_access_lock); // Lock to safely write output
    FILE *otpt = fopen("output-reader-pref.txt", "a");
    if (otpt) write_num_readers(otpt);
    sem_post(&file_access_lock);

    FILE *shared_file = fopen("shared-file.txt", "a"); // Writing to shared file
    if (shared_file) write_hello_world(shared_file);

    sem_post(&resource_access_lock);  // Unlock resource for other writers or readers
    return NULL;
}

int check_num_args(int argc){
    if (argc != 3) return 0;
    return 1;
}

int main(int argc, char **argv) 
{
    int check_args = check_num_args(argc);
    if (!check_args) return 1;

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    pthread_t readers[n], writers[m];

    // Initialize semaphores
    sem_init(&read_counter_lock, 0, 1);
    sem_init(&resource_access_lock, 0, 1);
    sem_init(&file_access_lock, 0, 1);

    // Create reader and writer threads
    int i=0;
    while (i<n){
        pthread_create(&readers[i], NULL, reader, NULL);
        i++;
    }
    i=0;
    while (i<m){
        pthread_create(&writers[i], NULL, writer, NULL);
        i++;
    }
    i=0;
    while (i<n){
        pthread_join(readers[i], NULL);
        i++;
    }
    i=0;
    while (i<m){
        pthread_join(writers[i], NULL);
        i++;
    }

    // Destroy semaphores
    sem_destroy(&resource_access_lock);
    sem_destroy(&read_counter_lock);
    sem_destroy(&file_access_lock);
    sem_destroy(&reader_checker);
    sem_destroy(&writer_checker);
    return 0;
}