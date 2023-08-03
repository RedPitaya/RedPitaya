import datetime


class Clog(object):
    """ Singleton object Log.
        To get instance type:
            Log = clog.Clog()
    """

    def __new__(cls):
        if not hasattr(cls, 'instance'):
            cls.instance = super(Clog, cls).__new__(cls)
        return cls.instance

    def create_log_record(self, source, data):
        record = "[" + datetime.datetime.now().strftime("%a, %d %b %Y %H:%M:%S") + "][" + source + "]: " + data
        return record

    def print_stdout(self, record):
        print(record)

    def print_logfile(self, record):
        try:
            log_file = open("log.txt", "a")
            log_file.write(record + '\n')
            log_file.close()
        except:
            pass

    def log(self, source, data):
        record = self.create_log_record(source, data)
        self.print_logfile(record)
        self.print_stdout(record)

    def log_stdout(self, source, data):
        record = self.create_log_record(source, data)
        self.print_stdout(record)

    def log_file(self, source, data):
        record = self.create_log_record(source, data)
        self.print_logfile(record)
