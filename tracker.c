#include "tracker.h"
#define __lu__ "trac.lu"
// start private functions

void tracker_init(int page_min, const char* filename) {
    FILE* tracker = fopen(filename, "w");
    fprintf(tracker, "%d", page_min);
    fclose(tracker);
    
    FILE* tracker_lu = fopen(__lu__, "w");
    fprintf(tracker, "1 2 3");
    fclose(tracker_lu);
}

short tracker_peekAvailablePage(const char* filename) {
    FILE* tracker = fopen(filename, "r");
    int temp;
    if (fscanf(tracker, "%d", &temp) == EOF) 
        printf("Memory corrupt\n");
    
    fclose(tracker);
    return temp;
}

short tracker_getAvailablePage(const char* filename) {
    int temp;
    FILE* r_tracker = fopen(filename, "r");
    if (fscanf(r_tracker, "%d", &temp) == EOF) {
        fclose(r_tracker);
        printf("Memory corrupt\n");
        exit(1);
    }
    fclose(r_tracker);

    FILE* w_tracker = fopen(filename, "w");
    fprintf(w_tracker, "%d", temp + 1);
    fclose(w_tracker);

    return temp;
}

void delete_tracker(const char* filename) {
    if (remove(filename) != 0)
        printf("\n\tCould not delete tracker!\n\n");
    
    if (remove(__lu__) != 0)
        printf("\n\tCould not delete tracker!\n\n");
}

int tracker_least_used() {
    int temp;
    FILE* tracker_lu = fopen(__lu__, "r");
    if (fscanf(tracker_lu, "%d", &temp) == EOF) 
        printf("Memory corrupt\n");
    
    fclose(tracker_lu);
    return temp;
}

void tracker_update_lu(int just_used) {
    int arr[3];
    FILE* rtracker_lu = fopen(__lu__, "r");
    for (short i = 0; i < 3; ++i) {
        fscanf(rtracker_lu, "%d", &arr[i]);
    }
    fclose(rtracker_lu);

    if (arr[0] == just_used) {
        arr[0] = arr[1];
        arr[1] = arr[2];
        arr[2] = just_used;
    } else if (arr[1] == just_used) {
        arr[1] = arr[2];
        arr[2] = just_used;
    }

    FILE* wtracker_lu = fopen(__lu__, "w");
    for (short i = 0; i < 3; ++i) {
        fprintf(wtracker_lu, "%d ", arr[i]);
    }
    fclose(wtracker_lu);
}