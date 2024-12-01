#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
        "    join <filename1> <filename2> - join two files by field 'id'\n"
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
    if(!f) {
        printf("error of opening file\n");
        return 2;
    }
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
    // сделаю сортировку по полю cost, а если цены равны, то по полям: id, count, primary
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

void JoinDumpSingle(StatData *p, size_t count, StatData **pJoined, size_t *countJoined) {
    // функция переносит все записи из массива p в массив pJoined, объединяя записи по полю id
    for(size_t i = 0; i < count; i++) {
        size_t j;
        for(j = 0; j < *countJoined; j++) {
            if((*pJoined)[j].id == p[i].id) {
                break;
            }
        }
        if(j == *countJoined) {
            (*countJoined)++;
            (*pJoined)[j] = p[i];  
        }
        else {
            (*pJoined)[j].count += p[i].count;
            (*pJoined)[j].cost += p[i].cost;
            (*pJoined)[j].primary &= p[i].primary;
            (*pJoined)[j].mode = (*pJoined)[j].mode > p[i].mode ? (*pJoined)[j].mode : p[i].mode;
        }
    }
}

int JoinDump(StatData *pStatData1, size_t count1, StatData *pStatData2, size_t count2, StatData **pStatData3, size_t *count3) {
    // Напишите функцию, объединяющую два массива StatData Произвольной длины так, чтобы масcиве -
    // результате id всех записей был уникален. При объединении записей с одинаковым id:
    // - поля count и cost должны складываться,
    // - поле primary должно иметь значение 0, если хотя бы в одном из элементов оно 0,
    // - поле mode должно иметь максимальное значение из двух представленных.
    // Записей с повторяющимся id может быть произвольное количество.
    // printf("line: %d\n", __LINE__);
    size_t size3 = (count1+count2)*sizeof(StatData);
    // printf("count1=%d, count2=%d, size=%d\n", count1, count2, size3);
    *pStatData3 = malloc(size3);
    //memset(*pStatData3, 0, size3);
    // printf("line: %d\n", __LINE__);

    if(!pStatData3) {
        printf("malloc error: not enough memory\n");
        return 1;
    }
    *count3 = 0;
    // printf("line: %d\n", __LINE__);
    JoinDumpSingle(pStatData1, count1, pStatData3, count3);
    // printf("line: %d\n", __LINE__);
    JoinDumpSingle(pStatData2, count2, pStatData3, count3);
    // printf("line: %d\n", __LINE__);
    return 0;
}

int main(int argc, char* argv[]) {
    srand(time(0));
    
    if(argc < 2) {
        showHelp();
        return 0;
    }

    if(!strcmp(argv[1], "random")) {
        if(argc >= 4) {
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
        if(argc >= 3) {
            StatData *p = NULL;
            size_t n = 0;
            int result = LoadDump(&p, &n, argv[2]);
            if(result) {
                if(p)
                    free(p);
                return result;
            }
            printDump(p, n);
            free(p);
            return 0;
        }
    }

    if(!strcmp(argv[1], "sort")) {
        if(argc >= 3) {
            StatData *p = NULL;
            size_t n = 0;
            int result = LoadDump(&p, &n, argv[2]);
            if(result) {
                if(p)
                    free(p);
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
    
    if(!strcmp(argv[1], "join")) {
        if(argc >= 4) {
            // можно было бы p1,p2 и n1,n2 засунуть в массив(-ы), код, возможно, стал бы чуть короче
            StatData *p1 = NULL, *p2 = NULL, *pJoined = NULL;
            size_t n1 = 0, n2 = 0, nJoined = 0;

            int result = LoadDump(&p1, &n1, argv[2]);
            if(result) {
                if(p1)
                    free(p1);
                return result;
            }
            printf("loaded data (1):\n");
            printDump(p1, n1);

            result = LoadDump(&p2, &n2, argv[3]);
            if(result) {
                if(p1)
                    free(p1);
                if(p2)
                    free(p2);
                return result;
            }
            printf("loaded data (2):\n");
            printDump(p2, n2);

            JoinDump(p1, n1, p2, n2, &pJoined, &nJoined);
            SortDump(pJoined, nJoined);
            printf("joined data:\n");
            printDump(pJoined, nJoined);

            free(p1);
            free(p2);
            free(pJoined);
            return 0;
        }
    }

    showHelp();
    return 0;
}
