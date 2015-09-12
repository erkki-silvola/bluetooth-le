#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "btle/central/connectionhandlerobserver.h"
#include "btle/central/connectionhandlerscanctrl.h"
#include "btle/central/connectionhandlerlinkctrl.h"
#include "btle/central/centralplugininterface.h"
#include "btle/device.h"
#include "btle/timer.h"

#include <vector>
#include <map>

namespace btle {
    namespace central {

        enum connectionhandler_options{
            /**
             * if connection direct is not set connection will be triggred from advertisement head
             * connection is direct ie will suspend connection handler
             * if no timeout has been set
             */
            CONNECTION_DIRECT                  = 0x01,
            CONNECTION_TIMEOUT                 = 0x02,
            DISCONNECTION_TIMEOUT              = 0x04,
            RECONNECTION_TRYES                 = 0x08,
            /**
             * Parallel connection option, in version 3.x.x onwards
             */
            EXCUTE_PARALLEL                    = 0x10
        };

        class connectionhandler;

        /**
         * function pointer declaration
         */
        typedef void (connectionhandler::*kConnectionHndlrState)( device&, int );


        class BTLE_API connectionhandler: public timercallback
        {
        public:
            
            connectionhandler();
            void setup(device_list* list,
                       connectionhandlerscanctrl* ctrl,
                       connectionhandlerlinkctrl* link_ctrl);
            
        private:

            // when new adv has been received
            void advertisement_head_received(device &dev);

            // client/user action
            void connect_device(device &dev );
            void disconnect_device( device& dev );

            // callbacks when device has been really connected or disconnected
            void device_connected(device &dev );
            void device_disconnected( device& dev );

        public:

            void set_option(connectionhandler_options option, int value = 0);
            void remove_option(connectionhandler_options option);
            void attach(connectionhandlerobserver* observer);
            void detach(connectionhandlerobserver* observer);

        protected:

            void change_device_state( device& dev, btle::connection_state state );
            device* any_in_state(btle::connection_state state);
            device* oldest_in_connection_park();
            
        private: // states

            void free( device& dev, int action );
            void connecting( device& dev, int action );
            void disconnecting( device& dev, int action );
            void change_state(kConnectionHndlrState state, device &dev);

        private:
            
            void timer_expired(timer* t);
            
        private: // private functions

            bool is_reconnection_needed(device& dev);

        private: // data

            kConnectionHndlrState current_;
            kConnectionHndlrState free_;
            kConnectionHndlrState connecting_;
            kConnectionHndlrState disconnecting_;

            device* current_device_;
            int reconnectiontryes_;
            std::map<connectionhandler_options,int> options_;
            std::vector<connectionhandlerobserver*> observers_;
            timer timer_;
            connectionhandlerscanctrl* scan_ctrl_;
            connectionhandlerlinkctrl* link_ctrl_;
            device_list* devices_;
            
            friend class collector;
        };
    }
}

#endif // CONNECTIONHANDLER_H

