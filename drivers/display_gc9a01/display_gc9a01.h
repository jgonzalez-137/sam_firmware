#pragma once

class Gc9a01 {
public:
    Gc9a01();
    // pines: mosi, clk, cs, dc, rst
    bool init(int mosi, int clk, int cs, int dc, int rst);
    bool isInitialized() const;
    // Scaffold: funciones mínimas para integración UI
    void clear();

private:
    bool initialized_;
    int mosi_;
    int clk_;
    int cs_;
    int dc_;
    int rst_;
};
