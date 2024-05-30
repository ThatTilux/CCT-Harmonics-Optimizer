#ifndef SILENT_LOG_HH
#define SILENT_LOG_HH

#include <rat/common/log.hh>

// Class to Log nothing
namespace rat { namespace cmn {

    class SilentLog : public Log {
    public:
        SilentLog() : Log(LogoType::NONE) {}

        // Override all methods to do nothing
        void set_num_indent(const int num_indent) override {}
        void msg(const int incr) override {}
        void newl() override {}
        int get_num_indent() override { return 0; }
        void hline(const int width, const char ch, const std::string& str1, const std::string& str2) override {}

        static std::shared_ptr<SilentLog> create() {
            return std::make_shared<SilentLog>();
        }

    private:
        // Override all msg methods to do nothing
        void msg(const char* format, ...) override {}
    };

}} // namespace rat::cmn

#endif // SILENT_LOG_HH
