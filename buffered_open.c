//
// Created by ran on 6/28/24.
// ran wurembrand 315366039
#include "buffered_open.h"

char* initialbuffers(){
    char* buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    if (!buffer){
        free(buffer);
        buffer = NULL;
        return NULL;
    }
    return buffer;
}

// Function to wrap the original open function
buffered_file_t *buffered_open(const char *pathname, int flags, ...){
    buffered_file_t* ret;
    char* writebuffer = initialbuffers();
    if(!writebuffer){
        perror("failed allocating mem");
        exit(-1);
    }
    char* readbuffer = initialbuffers();
    if(!readbuffer){
        free(writebuffer);
        perror("failed allocating mem");
        exit(-1);
    }
    ret =(buffered_file_t *)malloc(sizeof(buffered_file_t));
    if(!ret){
        free(writebuffer);
        free(readbuffer);
        perror("failed allocating mem");
        exit(-1);
    }
    ret->flags = flags;
    ret->preappend = (flags & O_PREAPPEND);
    ret->read_buffer_size = BUFFER_SIZE;
    ret->write_buffer_size = BUFFER_SIZE;
    ret->read_buffer_pos = 0;
    ret-> write_buffer_pos = 0;
    ret->write_buffer = writebuffer;
    ret->read_buffer = readbuffer;
    ret->fd = open(pathname,flags,0644);
    if(ret->fd < 0){
        free(writebuffer);
        free(readbuffer);
        perror("failed open dir");
        exit(-1);
    }
    return ret;
}


// Function to flush the buffer to the file
int buffered_flush(buffered_file_t *bf){
    ssize_t bytes_wrote = write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
    if ( bytes_wrote != bf->write_buffer_pos) {
        perror("Failed to writing to dest");
        return -1;
    }
    memset(bf->write_buffer, '\0', BUFFER_SIZE);
    bf->write_buffer_pos = 0;
    return bytes_wrote;
}

ssize_t handle_write(buffered_file_t *bf, const void *buf, size_t count) {


    const char *tmp = (char *)buf;
    size_t toCopy = 0;
    size_t available_space = BUFFER_SIZE - bf->write_buffer_pos;
    size_t currIndex = 0;
    if (count == 0) {
        //printf("No data to write.\n");
        return 0;
    }
    while (currIndex <= count) {
        toCopy = (available_space > (count - currIndex)) ? (count - currIndex) : available_space;
        if (available_space == 0) {
            //printf("Buffer full. Flushing...\n");
            buffered_flush(bf);
            bf->write_buffer_pos = 0;  // Reset position after flushing
            available_space = BUFFER_SIZE;
            if(currIndex == count){
                return currIndex;
            }
            continue;
        }
        if(toCopy == 0){
            return currIndex;
        }
        strncpy(bf->write_buffer + bf->write_buffer_pos, tmp + currIndex, toCopy);
        //printf("Data written to buffer: %s\n", bf->write_buffer + bf->write_buffer_pos);
        bf->write_buffer_pos += toCopy;
        available_space -= toCopy;
        currIndex += toCopy;
    }
    return currIndex;  // Return the number of bytes written
}

// Helper function to read the entire file into a memory buffer
//char* read_full_file(int fd, size_t *length) {
//    char *buffer = NULL;
//    ssize_t bytes_read;
//    size_t total_read = 0;
//    size_t buffer_size = 4096;
//    buffer = malloc(buffer_size);
//
//    if (!buffer) {
//        perror("Failed to allocate buffer");
//        return NULL;
//    }
//
//    lseek(fd, 0, SEEK_SET);  // Ensure we start reading from the start of the file
//    while ((bytes_read = read(fd, buffer + total_read, buffer_size - total_read)) > 0) {
//        total_read += bytes_read;
//        if (total_read == buffer_size) {
//            buffer_size *= 2;
//            char *new_buffer = realloc(buffer, buffer_size);
//            if (!new_buffer) {
//                free(buffer);
//                perror("Failed to expand buffer");
//                return NULL;
//            }
//            buffer = new_buffer;
//        }
//    }
//    *length = total_read;
//    return buffer;
//}
//


// Improved handle_preappend_write with better error handling and efficiency
ssize_t handle_preappend_write(buffered_file_t *bf, const void *buf, size_t count) {
    lseek(bf->fd, 0, SEEK_END); // Go to the end of the file to avoid overwriting
    size_t file_size = lseek(bf->fd, 0, SEEK_CUR); // Get file size
    char *temp_buf = malloc(file_size); // Allocate buffer to hold existing data
    if (!temp_buf) {
        perror("Allocation failed");
        return -1;
    }

    lseek(bf->fd, 0, SEEK_SET); // Reset to start to read existing data
    if (read(bf->fd, temp_buf, file_size) != file_size) {
        perror("Read failed");
        free(temp_buf);
        return -1;
    }

    lseek(bf->fd, 0, SEEK_SET); // Reset to start to write new data

    ssize_t bytes_written = handle_write(bf, buf, count); // Write new data
    if (bytes_written < 0) {
        perror("Write new data failed");
        free(temp_buf);
        return -1;
    }

    if (handle_write(bf, temp_buf, file_size) != file_size) { // Append old data
        perror("Write existing data failed");
        free(temp_buf);
        return -1;
    }

    free(temp_buf);
    return bytes_written;// Return total bytes written
}


ssize_t buffered_write(buffered_file_t *bf, const void *buf, size_t count) {
    if (bf->preappend) {
        // Handle prepending
        return handle_preappend_write(bf, buf, count);
    } else{
        return handle_write(bf,buf,count);
    }
}

ssize_t buffered_read(buffered_file_t *bf, void *buf, size_t count) {
    if (count == 0) return 0;  // No space to read into
    buffered_flush(bf);
    char *tmp = (char *)buf;
    size_t toCopy = 0;
    ssize_t bytesRead = 0;
    size_t currIndex = 0;

    while (currIndex < count) {  // Adjust condition to use the full count
        if (bf->read_buffer_pos == BUFFER_SIZE || bf->read_buffer_pos == 0) {
            bytesRead = read(bf->fd, bf->read_buffer, BUFFER_SIZE);
            if (bytesRead < 0) {
                perror("failed reading from file");
                return -1;  // Return -1 on read error
            }
            if (bytesRead == 0) {
                break;  // Exit the loop as no more data can be read
            }
            bf->read_buffer_pos = 0;  // Reset buffer position
        }

        size_t available_toRead = bytesRead - bf->read_buffer_pos;
        toCopy = (available_toRead > (count - currIndex)) ? (count - currIndex) : available_toRead;

        // Copy data from read buffer to user buffer
        memcpy(tmp + currIndex, bf->read_buffer + bf->read_buffer_pos, toCopy);
        bf->read_buffer_pos += toCopy;
        currIndex += toCopy;
    }

    // It's up to the caller to add a null terminator if needed
    return currIndex;  // Return number of bytes actually read into user buffer
}

// Function to close the buffered file
int buffered_close(buffered_file_t *bf){
    if(bf->write_buffer_pos !=0){
        buffered_flush(bf);
    }

    return close(bf->fd);
}
