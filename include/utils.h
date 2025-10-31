/**
    * @file: utils.h
    * @author: Dmytro Kovalchuk
    *
    * This header file declares utility functions used across
    * the server application.
    *
    * It primarily includes signal handling function that allow
    * the server to terminate safely upon receiving SIGINT.
*/

#ifndef UTILS_H
#define UTILS_H

/**
    * @brief Handles SIGINT (Ctrl+C) signal for safe shutdown.
    *
    * This function is triggered when the server process receives
    * a SIGINT signal. It ensures that all that the server shuts down
    * in a safely manner.
    *
    * @param sig The signal number received by the process.
*/
void handle_sigint(int sig);

#endif