The function "rlestreamdecode" decodes an RLE-encoded file piece by piece, returning integer arrays of the requested size.

This function accepts three arguments:
- filepath: a string containing the file path to the compressed/encoded data file
- seg_len: the number of integers requested (a smaller number may be returned if there are not enough remaining in the file)
- num_res: a pointer to a variable whose value the function will set to the true number of elements returned

Return value: an integer array (int *) containing the results of the decompression. Calling the function will return results from the last recorded point in the file. If the end of the file is reached during one function call, the next call to this function will start over from the beginning of the file.