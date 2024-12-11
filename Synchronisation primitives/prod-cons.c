#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int buffer[100];
int fill = 0;
int use = 0;
int cnt = 0;
int done = 0; 

pthread_mutex_t buff_mutex;
pthread_cond_t empty, fill_cond;

void put(int value) {
    buffer[fill] = value;
    fill++;
    fill%=100;
    cnt++;
}

int get() {
    int x = buffer[use];
    use++;
    use%=100;
    cnt--;
    return x;
}

void *producer(void *arg) {
    FILE *ipt = fopen("input-part1.txt", "r");
    if (ipt == NULL) {
        exit(1);
    }

    int value;
    while (fscanf(ipt, "%d", &value) == 1) {
        if (value == 0) break;
        pthread_mutex_lock(&buff_mutex);
        while (cnt == 100) pthread_cond_wait(&empty, &buff_mutex);
        put(value);
        pthread_cond_signal(&fill_cond);
        pthread_mutex_unlock(&buff_mutex);
    }
    //production is done
    pthread_mutex_lock(&buff_mutex);
    done = 1;
    pthread_cond_signal(&fill_cond);  // Wake up consumer 
    pthread_mutex_unlock(&buff_mutex);
    fclose(ipt);
    return NULL;
}

void *consumer(void *arg) {
    FILE *output = fopen("output-part1.txt", "w");
    if (output == NULL) {
        perror("Error opening output file");
        exit(1);
    }
    while (1) {
        pthread_mutex_lock(&buff_mutex);
        while (cnt == 0 && !done) pthread_cond_wait(&fill_cond, &buff_mutex); // Buffer is empty, but producer may still be producing
        // Check if producer has finished and buffer is empty
        if (cnt == 0 && done) {
            pthread_mutex_unlock(&buff_mutex);
            break;  // Exit consumer loop
        }
        int x = get();
        // Print consumed item and buffer state to the output file
        fprintf(output, "Consumed:[%d],Buffer-State:[", x);
        for (int i = 0; i < cnt; i++) {
            fprintf(output, "%d", buffer[(use + i) % 100]);
            if (i < cnt - 1) fprintf(output, ",");
        }
        fprintf(output, "]\n");
        fflush(output);
        pthread_cond_signal(&empty);  // Signal the producer
        pthread_mutex_unlock(&buff_mutex);
    }
    fclose(output);
    return NULL;
}

int main() {
    pthread_t prod_thd, cons_thd;

    pthread_mutex_init(&buff_mutex, NULL);
    pthread_cond_init(&empty, NULL);
    pthread_cond_init(&fill_cond, NULL);

    pthread_create(&prod_thd, NULL, producer, NULL);
    pthread_create(&cons_thd, NULL, consumer, NULL);

    pthread_join(prod_thd, NULL);
    pthread_join(cons_thd, NULL);

    pthread_mutex_destroy(&buff_mutex);
    pthread_cond_destroy(&empty);
    pthread_cond_destroy(&fill_cond);

    return 0;
}