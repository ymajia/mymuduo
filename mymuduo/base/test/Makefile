LOG_OBJS = log_test.cc ../AsyncLogging.cc ../Logger.cc ../Timestamp.cc ../LoggerStream.cc ../Thread.cc ../CurrentThread.cc

log_test: $(LOG_OBJS)
	g++ $^ -o log_test -lpthread

log_debug: $(LOG_OBJS)
	g++ $^ -g -o log_test -lpthread -Wl --no-as-needed

clean:
	rm log_test