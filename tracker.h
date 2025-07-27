#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/// @brief Will initialize the tracker, which increments the value of the current page everytime it is instructed to do so
/// @param page_min is the smallest value a page can have
/// @param filename is the file where the tracker will be stored
void tracker_init(int page_min, const char* filename);

/// @brief Peek at the next available page
/// @param filename name of the file where the tracker is stored
/// @return the value of the next available page
short tracker_peekAvailablePage(const char* filename);

/// @brief Get the next available page, and increment the value for next time
/// @param filename name of the file where the tracker is stored
/// @return the value of the next available page
short tracker_getAvailablePage(const char* filename);

/// @brief Remove tracker
/// @param filename name of the file where the tracker is stored
void delete_tracker(const char* filename);

/// @brief Return leased used page on RAM
/// @param filename name of the file where the tracker is stored
/// @return leased used page on RAM
int tracker_least_used();

/// @brief Tell the tracker which page was just used to keep the least used value up-to-date
/// @param filename name of the file where the tracker is stored
/// @param just_used the pageID of the page just accessed by the user
void tracker_update_lu(int just_used);