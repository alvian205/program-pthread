#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock_cetak = PTHREAD_MUTEX_INITIALIZER;

/* Setiap thread punya struct sendiri: berisi INPUT dan tempat menyimpan HASIL.
 * Struct dibuat di main, jadi tetap ada setelah thread selesai. */
typedef struct {
    int n;                       /* input  */
    unsigned long long hasil;    /* output */
} TugasFaktorial;

typedef struct {
    int jumlah;                  /* input  */
    unsigned long long suku_terakhir;  /* output */
    unsigned long long total;          /* output: jumlah semua suku */
} TugasFibonacci;

typedef struct {
    int batas;                   /* input  */
    int banyak;                  /* output: berapa bilangan prima */
    int terbesar;                /* output: prima terbesar */
} TugasPrima;

/* ---------- Thread 1: Faktorial ---------- */
void *hitung_faktorial(void *arg) {
    TugasFaktorial *t = (TugasFaktorial *)arg;
    unsigned long long hasil = 1;

    for (int i = 1; i <= t->n; i++) {
        hasil *= i;
        usleep(100000);
    }
    t->hasil = hasil;            /* simpan hasil ke struct */
    return NULL;
}

/* ---------- Thread 2: Fibonacci ---------- */
void *tampilkan_fibonacci(void *arg) {
    TugasFibonacci *t = (TugasFibonacci *)arg;
    unsigned long long a = 0, b = 1, next, total = 0;

    for (int i = 0; i < t->jumlah; i++) {
        pthread_mutex_lock(&lock_cetak);
        printf("[Thread 2] Fibonacci ke-%d = %llu\n", i + 1, a);
        pthread_mutex_unlock(&lock_cetak);

        t->suku_terakhir = a;
        total += a;
        next = a + b;
        a = b;
        b = next;
        usleep(100000);
    }
    t->total = total;
    return NULL;
}

/* ---------- Thread 3: Bilangan Prima ---------- */
int cek_prima(int x) {
    if (x < 2) return 0;
    for (int i = 2; i * i <= x; i++)
        if (x % i == 0) return 0;
    return 1;
}

void *cari_prima(void *arg) {
    TugasPrima *t = (TugasPrima *)arg;
    int banyak = 0, terbesar = 0;

    for (int i = 2; i <= t->batas; i++) {
        if (cek_prima(i)) {
            banyak++;
            terbesar = i;
            pthread_mutex_lock(&lock_cetak);
            printf("[Thread 3] Bilangan prima: %d\n", i);
            pthread_mutex_unlock(&lock_cetak);
            usleep(100000);
        }
    }
    t->banyak = banyak;
    t->terbesar = terbesar;
    return NULL;
}

/* ---------- Main ---------- */
int main(void) {
    pthread_t t1, t2, t3;
    TugasFaktorial f = {0};
    TugasFibonacci b = {0};
    TugasPrima     p = {0};

    printf("Masukkan bilangan untuk faktorial (0-20)     : ");
    if (scanf("%d", &f.n) != 1 || f.n < 0 || f.n > 20) {
        printf("Input tidak valid (harus 0-20 agar tidak overflow).\n");
        return 1;
    }
    printf("Masukkan jumlah suku deret Fibonacci (1-90)  : ");
    if (scanf("%d", &b.jumlah) != 1 || b.jumlah < 1 || b.jumlah > 90) {
        printf("Input tidak valid (harus 1-90 agar tidak overflow).\n");
        return 1;
    }
    printf("Masukkan batas atas bilangan prima           : ");
    if (scanf("%d", &p.batas) != 1 || p.batas < 2) {
        printf("Input tidak valid (minimal 2).\n");
        return 1;
    }

    printf("\n=== Menjalankan 3 thread secara bersamaan ===\n\n");

    if (pthread_create(&t1, NULL, hitung_faktorial, &f) != 0 ||
        pthread_create(&t2, NULL, tampilkan_fibonacci, &b) != 0 ||
        pthread_create(&t3, NULL, cari_prima, &p) != 0) {
        perror("Gagal membuat thread");
        return 1;
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    /* Hanya setelah join, hasil aman dibaca oleh main */
    printf("\n=== RINGKASAN HASIL AKHIR ===\n");
    printf("Thread 1 (Faktorial) : %d! = %llu\n", f.n, f.hasil);
    printf("Thread 2 (Fibonacci) : suku terakhir = %llu, jumlah %d suku = %llu\n",
           b.suku_terakhir, b.jumlah, b.total);
    printf("Thread 3 (Prima)     : %d bilangan prima sampai %d (terbesar: %d)\n",
           p.banyak, p.batas, p.terbesar);

    pthread_mutex_destroy(&lock_cetak);
    printf("\n=== Semua thread selesai ===\n");
    return 0;
}
