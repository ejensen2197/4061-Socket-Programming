# 4061-Socket-Programming

Project Group: Group 12

Names and x500: Noah Zahrt (zahrt003), Eric Nguyen (nguy4911), Ethan Jensen (jens2543)

Computer for testing: csel-kh1250-01.cselabs.umn.edu

Individual Contributions: 
init and accept_connections - Ethan
send_file_to_client , get_request_server , setup_connection - Noah
send_file_to_server and receive_file_from_server - Eric

No Changes to makefile

Make sure to add "img" folder to output folder


Through the use of dispatcher and worker functions in server.c we can allow for 
the program to work on requests in parrallel.

dispatch function{
    while true{
        client_fd = accepted connection #accept a new connection
        file_buffer = get from server (client_fd) #read the file
        lock
        while the queue is full{
            wait #if the queue is full, wait
        }

        enqueue the request(file_buffer, client_fd)

        signal to workers 
        unlock
    }
}

worker function{
    while true{
        lock
        while queue is empty{
            wait #if there are no requests, then wait for one to do work
        }
        request = dequeued request from queue #get the request

        signal to dispatchers
        unlock
        process the request that we just dequeued #do work on the request
    }
}
