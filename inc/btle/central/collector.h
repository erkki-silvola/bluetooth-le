#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <sstream>
#include <mutex>

#include "btle/uuid.h"
#include "btle/device.h"
#include "btle/bda.h"
#include "btle/scanfilterbase.h"
#include "btle/central/centralplugininterface.h"
#include "btle/central/centralpluginobserver.h"
#include "btle/central/connectionhandler.h"

namespace btle {
    namespace central {

        enum collector_mode{
            COLLECTOR_POWER_SAVE_OFF = 0x00,
            COLLECTOR_POWER_SAVE_ON  = 0x01
        };

        /**
         * @brief The collector class, main interface class for lib central usage
         */
        class BTLE_API collector: public  centralpluginobserver,
                                  private connectionhandlerobserver,
                                  private connectionhandlerscanctrl,
                                  private connectionhandlerlinkctrl
        {
        public:

            collector();
            ~collector();
            
        public: // plugin start/stop etc...
            
            const std::vector<std::string>& plugins_available() const;
            int start(const std::string& plugin_name);
            int auto_start();
            void stop();
            
        public: // scannning

            void add_scan_filter(scanfilterbase* filter);
            void remove_scan_filter(scanfilterbase* filter);
            void clear_scan_filters();

            void start_scan();
            void stop_scan();

        public: // settings related apis

            connectionhandler& connection_handler();

        public:
            
            device_list devices_in_order(int rssi_limit,bool ascent=false);
            
        public:

            void set_auto_read_values(const uuid_list& list);
            void set_auto_notify_values(const uuid_list& list);

        public: // device connection/disconnection

            void connect_device(const bda& addr);
            void connect_device(device& dev);
            void disconnect_device(device& dev);
            void disconnect_device(const bda& addr);

        public:

            void set_mode(collector_mode mode);

        public: // device gatt operations

            void read_characteristic_value(device& dev, const uuid& uid);
            void read_characteristic_value(device& dev, const uuid_pair& pair);
            void write_characteristic_value(device& dev, const uuid& uid, const std::string &data, bool write_with_out_resp = false);
            void write_characteristic_value(device& dev, const uuid_pair& pair, const std::string& data, bool write_with_out_resp = false);
            void set_characteristic_notify(device& dev, const uuid& uid, bool notify);
            void set_characteristic_notify(device& dev, const uuid_pair& pair, bool notify);

        public: // connection parameters

            void update_connection_parameters(btle::device& dev, const connectionparameters& params);

        public: // device out stream operations

            void write_file(btle::device& dev, std::ostream& stream, int identifier=0);

        private: // from connectionhandler

            void device_state_changed(btle::device& dev);
                                      
        private: // scan controller
                                      
            void aquire_start_scan();
            void aquire_stop_scan();

        private: //
            
            void aquire_connect_device(btle::device& dev);
            void aquire_disconnect_device(btle::device& dev);
            void aquire_cancel_pending_connection(btle::device& dev);
            
        private: // from observer

            void plugin_state_changed(plugin_state state);
            void new_device_discovered(device& dev,adv_fields& fields,int rssi);
            void device_discovered(device& dev, adv_fields& fields, int rssi);

        private:
            void process_device_discovered(device& dev, adv_fields& fields, int rssi, bool new_device=false);

        private: // from observer
            void device_connected(device& dev);
            void device_disconnected(device& dev);
            void device_services_discovered(device& dev, service_list& services, const error& err);
            void device_characteristics_discovered(device& dev, service& srv, chr_list& chrs, const error& err);
            void device_characteristic_read(device& dev, service& srv, characteristic& chr, std::string& data, const error& err);
            void device_characteristic_written(device& dev, service& srv, characteristic& chr, const error& err);
            void device_characteristic_nofication_state_changed(device& dev, service& srv, characteristic& chr, bool notify, const error& err);
            void device_descriptor_written(device& dev, service& srv, characteristic& chr, descriptor& desc, const error& err);
            void device_characteristic_notify_data_updated(device& dev, service& srv, characteristic& chr, std::string &data);
            void device_rssi_read(device& dev, int rssi);
            void device_services_invalidated(device& dev);
        public:
            virtual void device_connection_parameters_updated(device& dev);

        public: // callbacks

            virtual void new_device_discovered_cb(device& dev);
            /**
             * @brief device_discovered_cb, whenever a device has been discovered during scanning
             * @param dev
             */
            virtual void device_discovered_cb(device& dev);

            /**
             * @brief device_service_value_updated_cb, called upon whenever gatt service has been updated
             * e.g. famous heart rate example:
             *
             *      switch( srv->service_uuid().uuid16bit() )
             *      {
             *          case HEART_RATE_SERVICE:
             *          {
             *              const hrservice* hr = ((const hrservice*)srv);
             *              printf("Heart rate is: %d", hr->hr_value());
             *              break;
             *          }
             *          etc...
             *      }
             *
             * @param dev
             * @param srv
             */
            virtual void device_service_value_updated_cb(device& dev, const gatt_services::gattservicebase* srv) = 0;

            /**
             * @brief device_service_value_updated_cb
             * @param dev
             * @param srv
             * @param chr
             * @param data
             */
            virtual void device_service_value_updated_cb(device& dev, const service& srv, const characteristic& chr, const std::string& data) = 0;
            virtual void device_characteristic_read_cb(device& dev, const service& srv, const characteristic& chrs, const std::string& data, const error& err)=0;
            virtual void device_state_changed_cb(device& dev)=0;
            virtual void device_gatt_service_discovered_cb(device& dev, const gatt_services::gattservicebase *srv);
            virtual void device_service_discovery_failed_cb(device& dev, const service_list& services, const error& err);
            virtual void device_characteristic_discovery_failed_cb(device& dev, const service& srv, const chr_list& chrs, const error& err);
            virtual void plugin_state_changed_cb(plugin_state state);
            virtual void device_btle_ftp_in_progress(device& dev, double progress, const std::string& data, int identifier);
            virtual void device_btle_ftp_out_progress(device& dev, double progress, int identifier);

        public:

            device* fetch_device(const bda& addr);

        private:

            uuid_list notify_uuids_;
            uuid_list read_uuids_;
            centralplugininterface* plugin_;
            std::vector<centralplugininterface*> plugins_;
            connectionhandler connectionhandler_;
            unsigned int flags_;
            scan_filters filters_;
            plugin_state state_;
            std::vector<std::string> plugins_available_;
            std::mutex mutex_;
        };
    }
}

#endif // COLLECTOR_H

