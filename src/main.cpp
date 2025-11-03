#include "application.h"

Application app;

int main(){
    app.setup();

    int ret = app.run();

    app.cleanup();

    return ret;
}
