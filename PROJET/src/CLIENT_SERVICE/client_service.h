#ifndef CLIENT_SERVICE_H
#define CLIENT_SERVICE_H

// Ici toutes les communications entre les services et les clients :
// - les deux tubes nommés pour la communication bidirectionnelle

// tubes nommés
//#define PIPE_S2C "pipe_s2c"  	// Service vers Client
//#define PIPE_C2S "pipe_c2s"	// Client vers Service

// Pipe service sum
#define PIPE_S2C_SUM "pipe_s2c_1" // Service vers Client
#define PIPE_C2S_SUM "pipe_c2s_1" // Client vers Service

// Pipe service comp
#define PIPE_S2C_COMP "pipe_s2c_1" // Service vers Client
#define PIPE_C2S_COMP "pipe_c2s_1" // Client vers Service

// Pipe service sigma
#define PIPE_S2C_SIGMA "pipe_s2c_2" // Service vers Client
#define PIPE_C2S_SIGMA "pipe_c2s_2" // Client vers Service

#endif
