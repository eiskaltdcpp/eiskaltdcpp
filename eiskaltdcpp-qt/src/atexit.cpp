
extern "C" {

    int __wrap_atexit(void *) {
        return 0;
    }

}
