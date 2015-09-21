#ifndef CENTRALPLUGININTERFACE_H
#define CENTRALPLUGININTERFACE_H

#include "btle/central/centralbtleftptransferinterface.h"
#include "btle/device.h"

namespace btle {
    namespace central {

        // forward declarations
        class centralpluginobserver;

        enum central_plugin_extras
        {
            /**
             * Plugin can update/inform connection parameters
             */
            PLUGIN_CONNECTION_PARAMETER_CAPABILITY = 0x01,
            /**
             * Plugin has caps. for security manager controlling for 3rd party
             */
            PLUGIN_SMP_FULL_CAPABILITY             = 0x02,
            /**
             * Plugin has caps. for security manager, pin code input
             */
            PLUGIN_SMP_PIN_CODE_INPUT_CAPABILITY   = 0x04,
            /**
             * Plugin has caps. for Gatt server i.e. 3rd party can optionally add an service
             */
            PLUGIN_GATT_SERVER_CAPABILITY          = 0x08,
            /**
             * Plugin has caps. for setting different kind of scan parameters ( (window and interval) or preset )
             */
            PLUGIN_SCAN_PARAMTERS_CAPABILITY       = 0x10
        };

        enum central_scan_parameters
        {
            /**
             * High duty scan should be atleast 80 % duty cycle
             */
            SCAN_HIGH_DUTY,
            /**
             * Medium duty should be around 50-80% duty cycle
             */
            SCAN_MEDIUM_DUTY,
            /**
             * Low duty should be < 50% duty cycle
             */
            SCAN_LOW_DUTY
        };
        
        class centralplugininterface: public centralbtleftptransferinterface{
        public:

            centralplugininterface(centralpluginobserver& observer);

            virtual ~centralplugininterface();

            /**
             * @brief features
             * @return @see central_plugin_extras, returns bit mask of extra features if any
             */
            virtual unsigned int features();

            /**
             * @brief name
             * @return
             */
            virtual std::string name() = 0;

            /**
             * @brief devices
             * @return
             */
            virtual std::vector<device*>& devices() = 0;

            /**
             * @brief allocate_new_device
             * @param addr
             * @return
             */
            virtual device* allocate_new_device(const bda& addr) = 0;

            /**
             * @brief start
             * @return
             */
            virtual int start() = 0;

            /**
             * @brief stop
             */
            virtual void stop() = 0;

            /**
             * @brief start_scan
             * @param param
             * @param services
             */
            virtual void start_scan( central_scan_parameters param = SCAN_HIGH_DUTY, const uuid_list* services = NULL ) = 0;

            /**
             * @brief stop_scan
             */
            virtual void stop_scan() = 0;

            /**
             * @brief connect_device
             * @param dev
             */
            virtual void connect_device(device& dev)=0;

            /**
             * @brief disconnect_device
             * @param dev
             */
            virtual void disconnect_device(device& dev)=0;

            /**
             * @brief cancel_pending_connection
             * @param dev
             */
            virtual void cancel_pending_connection(device& dev)=0;
            
            /**
             * @brief discover_services
             * @param dev
             */
            virtual void discover_services(device& dev)= 0;
            
            /**
             * @brief discover_characteristics
             * @param dev
             */
            virtual void discover_characteristics(device& dev, const service& srv)= 0;

            /**
             * @brief read_characteristic_value
             */
            virtual void read_characteristic_value(device& dev,const service& srv, const characteristic& chr) = 0;

            /**
             * @brief write_characteristic_value
             * @param dev
             * @param srv
             * @param chr
             * @param data
             * @param type
             */
            virtual void write_characteristic_value(device& dev,const service& srv, const characteristic& chr, const std::string& data, characteristic_properties type) = 0;

            /**
             * @brief set_characteristic_notify, WILL BE DEPRECATED
             * @param dev
             * @param srv
             * @param chr
             */
            virtual void set_characteristic_notify(device& dev,const service& srv, const characteristic& chr, bool notify) = 0;

            /**
             * @brief write_descriptor
             * @param dev
             * @param srv
             * @param chr
             * @param desc
             * @param notify
             */
            virtual void write_descriptor(device& dev, const service& srv, const characteristic& chr, descriptor& desc, bool notify) = 0;

            /**
             * @brief update_parameters
             * @param dev
             * @param params
             */
            virtual void update_parameters(device& dev, const connectionparameters& params);

        protected:

            centralpluginobserver& observer_;
            std::vector<device*> devices_;
        };
    }
}

#endif // CENTRALPLUGININTERFACE_H

