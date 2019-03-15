#include <Homie.h>
#include <ota_wrapper.h>

HomieNode flowNode("flow", "Flow", "flow");
//HomieNode pulseRatio ("pulse-ratio", "pulse-ratio");
//HomieNode rawFlowNode("")

bool broadcastHandler(const String &level, const String &value)
{
    Homie.getLogger() << "Received broadcast level " << level << ": " << value << endl;
    return true;
}

const int SENSOR_PIN = D5;
unsigned long flow_count = 0;
unsigned long flow_ratio = 1000;

ICACHE_FLASH_ATTR
void flow_meter_isr()
{
    ++flow_count;
}

const int FLOW_INTERVAL = 1;

unsigned long lastFlowSent = 0;

void loopHandler()
{
    if (millis() - lastFlowSent >= FLOW_INTERVAL * 1000UL || lastFlowSent == 0)
    {

        unsigned long current_flow_count = flow_count;
        flow_count = 0;

        float flow = ((float)current_flow_count) / ((float)flow_ratio);
        flowNode.setProperty("litres").send(String(flow));
    }
}

void setup()
{
    Serial.begin(9600);
    Serial << endl
           << endl;
    Homie_setFirmware("flow-meter", "1.0.0");
    Homie.setBroadcastHandler(broadcastHandler);
    Homie.setLoopFunction(loopHandler);

    flowNode.advertise("litres").setName("Litres per second").setDatatype("float").setUnit("l/sec");

    Homie.setup();

    ota_setup();

    attachInterrupt(SENSOR_PIN, flow_meter_isr, RISING);
}

void loop()
{
    Homie.loop();

    ota_handle();
}