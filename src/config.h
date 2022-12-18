// Max Size of Memory
#define MSIZE 0x400

// api init_mode (PART II related) PIPE_MODE/SEQ_MODE
#define INIT_MODE SEQ_MODE

// ---------- Branch Predict Settings ----------

// Never_Taken, Always_Taken, Branch_Predict_Bimodal
#define BPStrategy Always_Taken

// output BP accuracy in stderr
// #define BPTEST

// Bimodal setting
#define PRED_BUF_SIZE 32
