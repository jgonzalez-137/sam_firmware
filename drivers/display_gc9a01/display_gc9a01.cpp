#include "display_gc9a01.h"
#include "esp_log.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "GC9A01";

Gc9a01::Gc9a01()
    : initialized_(false), mosi_(-1), clk_(-1), cs_(-1), dc_(-1), rst_(-1) {}

bool Gc9a01::init(int mosi, int clk, int cs, int dc, int rst)
{
    mosi_ = mosi;
    clk_ = clk;
    cs_ = cs;
    dc_ = dc;
    rst_ = rst;
    // Configurar GPIO de control
    gpio_set_direction((gpio_num_t)rst_, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)dc_, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)cs_, GPIO_MODE_OUTPUT);

    // Reset hardware: pulse RST low -> high
    gpio_set_level((gpio_num_t)rst_, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level((gpio_num_t)rst_, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // Configurar bus SPI
    spi_bus_config_t buscfg{};
    buscfg.mosi_io_num = (gpio_num_t)mosi_;
    buscfg.miso_io_num = -1;
    buscfg.sclk_io_num = (gpio_num_t)clk_;
    buscfg.quadhd_io_num = -1;
    buscfg.quadwp_io_num = -1;
    esp_err_t err = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %d", err);
        return false;
    }

    spi_device_interface_config_t devcfg{};
    devcfg.clock_speed_hz = 1000 * 1000 * 10; // 10 MHz default
    devcfg.spics_io_num = cs_;
    devcfg.queue_size = 1;
    devcfg.flags = SPI_DEVICE_HALFDUPLEX;

    spi_device_handle_t handle;
    err = spi_bus_add_device(VSPI_HOST, &devcfg, &handle);
    if (err != ESP_OK && err != ESP_ERR_NO_MEM) {
        ESP_LOGW(TAG, "spi_bus_add_device returned %d (may be already added)", err);
    }

    initialized_ = true;
    ESP_LOGI(TAG, "GC9A01 init OK (SPI configured)");
    return initialized_;
}

bool Gc9a01::isInitialized() const { return initialized_; }

void Gc9a01::clear()
{
    if (!initialized_) return;
    // Scaffold: implementar limpieza de framebuffer
    ESP_LOGI(TAG, "GC9A01 scaffold clear called");
}
