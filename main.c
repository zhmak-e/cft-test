#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct StatData {
    long id;
    int count;
    float cost;
    unsigned int primary:1;
    unsigned int mode:3;
} StatData;

void showHelp() {
    printf("usage:\n"
        "    random <N> <filename> - writes N random records of type StatData to file\n"
        "    load <filename> - load records of type StatData from file and print them\n"
        "    sort <filename> - load records from file, sort them and print\n"
        "    \n");
}

void printDump(StatData *pStatData, size_t count) {
    // заполняет массив p случайными данными в количестве count записей
    for(size_t i = 0; i < count; i++) {
        printf("id=0x%08x, count=%i, cost=%.3e, primary=%c, mode=%03b\n",
            pStatData[i].id, pStatData[i].count, pStatData[i].cost, pStatData[i].primary?'y':'n', pStatData[i].mode);
    }
}

void fillDump(StatData *pStatData, size_t count) {
    // заполняет массив p случайными данными в количестве count записей
    for(size_t i = 0; i < count; i++) {
        pStatData[i].id = rand()%10; // long может принимать значения от -4 млрд до 4 млрд, но здесь просто демонстрация, поэтому будет меньше чисел (от 0 до 2 млрд)
        pStatData[i].count = rand() % 10;//sizeof(int);
        // pStatData[i].cost = (float)rand() / 1000.0; // значения от балды
        pStatData[i].cost = (rand()%20);
        pStatData[i].primary = rand() % 2;
        pStatData[i].mode = rand() % 8;
    }
}

int StoreDump(StatData *pStatData, size_t count, const char* filename) {
    // Напишите функцию для сохранения массива StatData (длина массива произвольная) данных в файл, путь
    // до которого передан в качестве аргумента.
    FILE *f = fopen(filename, "wb");
    if(!f) {
        printf("error of opening file\n");
        return 2;
    }
    size_t n = fwrite(pStatData, sizeof(StatData), count, f);  // или тут в текстовом человекочитаемом виде надо было писать в файл?
    if(n != count) {
        printf("error of writing file\n");
        return 3;
    }
    return fclose(f); // не сталкивался я с ошибками закрытия файлов на своей практике,
                      // так что и тут, думаю, всё будет ок и не буду обрабатывать эту ошибку :)
}

int LoadDump(StatData **pStatData, size_t *count, const char* filename) {
    // Напишите функцию для чтения массива записей StatData из файла (файл сформирован функцией
    // StoreDump).
    FILE *f = fopen(filename, "rb");
    fseek(f, 0L, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    if(file_size % sizeof(StatData)) {
        printf("incorrect file - it's size is not a multiple of structure size StatData\n");
        return 4;
    }
    *count = file_size / sizeof(StatData);
    *pStatData = malloc(file_size);
    if(!*pStatData) {
        printf("malloc error: not enough memory\n");
        return 1;
    }
    size_t n = fread(*pStatData, sizeof(StatData), *count, f);
    if(n != *count) {
        printf("error of reading file");
        return 5;
    }
    return 0;
}

int compareStatData(const void *pv1, const void *pv2) {
    // сделаю сортировку по полю cost, и, если цены равны, то по полям: id, count, primary
    // по полю mode сортировки нет
    const StatData *p1 = pv1, *p2 = pv2;
    return p1->cost - p2->cost ? (p1->cost - p2->cost)
        : p1->id - p2->id ? (p1->id - p2->id)
        : p1->count - p2->count ? (p1->count - p2->count)
        : p1->primary - p2->primary;
}

void SortDump(StatData *pStatData, size_t count) {
    // Напишите функцию, сортирующую два массива StatData в порядке возрастания значения поля cost.
    // вероятно, в задании опечатка и надо сортировать лишь один массив?
    // если надо сортировать два массива, то достаточно лишь вызвать функцию сортировки дважды - для каждого массива
    // если надо объединить массивы вместе с сортировкой, то не описаны правила объединения
    // к тому же, далее в тестовом сортировка используется для одного (уже объединённого) массива
    // поэтому реализую сортировку одного массива

    // воспользуюсь стандартной реализацией быстрой сортировки (если надо, можно написать свою реализацию):
    qsort(pStatData, count, sizeof(StatData), compareStatData);
}

// JoinDump()
// Напишите функцию, объединяющую два массива StatData Произвольной длины так, чтобы масcиве -
// результате id всех записей был уникален.

int main(int argc, char* argv[]) {
    printf("RAND_MAX=%d\n", RAND_MAX);
    printf("sizeof(StatData)=%u\n", sizeof(StatData));
    if(argc < 2) {
        showHelp();
        return 0;
    }

    if(!strcmp(argv[1], "random")) {
        if(argc == 4) {
            size_t n = atoi(argv[2]);
            if(n > 0) {
                StatData *p = malloc(n * sizeof(StatData));
                if(!p) {
                    printf("malloc error: not enough memory\n");
                    return 1;
                }
                //memset(p, 0, n * sizeof(StatData));
                fillDump(p, n);
                printDump(p, n);
                int res = StoreDump(p, n, argv[3]);
                free(p); // в принципе, можно было бы и не освобождать, всё
                         // равно программа заканчивается, всё само подчистится
                return res;
            }
        }
    }

    if(!strcmp(argv[1], "load")) {
        if(argc == 3) {
            StatData *p = NULL;
            size_t n = 0;
            int result = LoadDump(&p, &n, argv[2]);
            if(result) {
                return result;
            }
            printDump(p, n);
            free(p); // в принципе, можно было бы и не освобождать, всё
                         // равно программа заканчивается, всё само подчистится
            return 0;
        }
    }

    if(!strcmp(argv[1], "sort")) {
        if(argc == 3) {
            StatData *p = NULL;
            size_t n = 0;
            int result = LoadDump(&p, &n, argv[2]);
            if(result) {
                return result;
            }
            printf("loaded data:\n");
            printDump(p, n);
            SortDump(p, n);
            printf("after sort:\n");
            printDump(p, n);
            free(p);
            return 0;
        }
    }
    
    showHelp();
    return 0;
}
