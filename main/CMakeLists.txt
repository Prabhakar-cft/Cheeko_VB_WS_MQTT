idf_component_register( SRCS 
                            "main.c"
                            "opus_wapper.c"
                            "mqtt_handler.c"
                            "led.c"
                            "wifi_manager.c"
                            "websocket_client.c"
                            "audio_session.c"
                            "button_handler.c"
                            "session_manager.c"
                    
                        INCLUDE_DIRS "."
                            REQUIRES 
                            esp_websocket_client 
                            opus 
                            vb6824 
                            mbedtls
                            mqtt
                            json
                            
                        PRIV_INCLUDE_DIRS 
                            "../components/opus/include" 
                            "../components/vb6824"

                        PRIV_REQUIRES 
                            driver 
                            esp_wifi 
                            nvs_flash 
                            lwip 
                            esp_netif 
                            esp_http_client 
                            esp_http_server)