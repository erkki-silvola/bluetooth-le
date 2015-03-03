#ifndef CENTRALPLUGININTERFACE_H
#define CENTRALPLUGININTERFACE_H

namespace btle {
    namespace central {
        class centralplugininterface{
        public:

            virtual int start() = 0;
            virtual void stop() = 0;
            virtual void start_scan() = 0;
            virtual void stop_scan() = 0;

        };
    }
}

#endif // CENTRALPLUGININTERFACE_H
