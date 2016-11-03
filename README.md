<b> Home Check Slack Bot </b>
-------------
for the Isobar Australia SlackHack 2016


• Implementation Concept is for a Home to interface via Slack
> - Monitor temperature, humidity and door open status
> - Remotely turn on & off a light and fan
> - Gas leakage detection alarm & Panic button alerts

•	Slack Integrations
> - 	Slash Command
> - 	Incoming Webhook

•	Code
> - Node / Express
> - Embedded C

•	Hardware 
> - Arduino board
> - 	Arduino GSM Shield
> - 	Sensors, Relays, voltage convertors



<b>High LevelProcess flow </b>


![Process flow](https://cloud.githubusercontent.com/assets/5301598/19955905/423c6f6c-a1da-11e6-9064-325f91437731.png)


<b> Hardware Block diagram</b>

![Process flow](https://cloud.githubusercontent.com/assets/5301598/19955907/44d96a68-a1da-11e6-8a93-520f6252f0e5.png)

Hardware SetUp:--
-------------

> - 	DHT22  Humidity and temperature sensor
> - 	Gas leakage Sensor/smoke Sensor(MQ2)
> - 	Door Sensor (borrowed from a printer)
> - 	Led light
> - 	CPU fan(12 V) 
