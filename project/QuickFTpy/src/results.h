/*
 * results.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef RESULTS_H
#define	RESULTS_H

#ifdef	__cplusplus
extern "C" {
#endif

// Define valores de resultados
#define RESULT_SUCCESS                                    0
#define RESULT_CONNECTION_ERROR                           -100
#define RESULT_UNDEFINED                                  -101
#define RESULT_CONFIG_ERROR                               -102
#define RESULT_INVALID_REQUEST                            -103
#define RESULT_INVALID_RESPONSE                           -104

#define RESULT_FILE_ACCESS_ERROR                          -105
#define RESULT_FILE_NOT_FOUND                             -106
#define RESULT_FILE_WRITE_ERROR                           -107
#define RESULT_FILE_READ_ERROR                            -108

#define RESULT_FILE_COMPRESS_ERROR                        -109
#define RESULT_FILE_DECOMPRESS_ERROR                      -110
#define RESULT_FILE_ENCODE_ERROR                          -111
#define RESULT_FILE_DECODE_ERROR                          -112
#define RESULT_FILE_DELETE_ERROR                          -113

#define RESULT_INVALID_DESTINATION_DIRECTORY              -114
#define RESULT_COULD_NOT_CREATE_DESTINATION_DIRECTORY     -115

// Define los mensajes de resultados
#define STR_RESULT_SUCCESS                                "SUCCESS____________"
#define STR_RESULT_CONNECTION_ERROR                       "CONNECTION_ERROR___"
#define STR_RESULT_UNDEFINED                              "UNDEFINED__________"
#define STR_RESULT_CONFIG_ERROR                           "CONFIG_ERROR_______"
#define STR_RESULT_INVALID_REQUEST                        "INVALID_REQUEST____"
#define STR_RESULT_INVALID_RESPONSE                       "INVALID_RESPONSE___"

#define STR_RESULT_FILE_ACCESS_ERROR                      "FILE_ACCESS_ERROR__"
#define STR_RESULT_FILE_NOT_FOUND                         "FILE_NOT_FOUND_____"
#define STR_RESULT_FILE_WRITE_ERROR                       "FILE_WRITE_ERROR___"
#define STR_RESULT_FILE_READ_ERROR                        "FILE_READ_ERROR____"

#define STR_RESULT_FILE_COMPRESS_ERROR                    "COMPRESS_ERROR_____"
#define STR_RESULT_FILE_DECOMPRESS_ERROR                  "DECOMPRESS_ERROR___"
#define STR_RESULT_FILE_ENCODE_ERROR                      "ENCODE_ERROR_______"
#define STR_RESULT_FILE_DECODE_ERROR                      "DECODE_ERROR_______"
#define STR_RESULT_FILE_DELETE_ERROR                      "DELETE_ERROR_______"

#define STR_RESULT_INVALID_DESTINATION_DIRECTORY          "DEST_DIR_INVALID___"
#define STR_RESULT_COULD_NOT_CREATE_DESTINATION_DIRECTORY "DEST_DIR_CREATE_ERR"

#ifdef	__cplusplus
}
#endif

#endif	// RESULTS_H
