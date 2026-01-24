#include "context.h"

void TContext::Stop() {
    End.store(true);
    WriteCv.notify_one();
    ReadCv.notify_one();
}

void TContext::Start() {
    Queue.clear();
    End.store(false);
}

bool TContext::IsEnd() {
    return End.load();
}
