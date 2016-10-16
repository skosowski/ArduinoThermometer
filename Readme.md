## Introduction
This is a contact thermometer with alerting function for temperature drop or rise below or above a given threshold. If alert condition occurs, an SMS message is sent to a given number.
Project uses three main components: 
* Arduino Uno Board 
* Arduino GSM module
* TI TMP102 digital thermometer 

## Mode of operation
send "T" to request current temperature
send "Off" to turn off alerting
Format of alerting is "A[l|m][+|-]xxx"
where l is less, m is more, + or - for positive or negative temperature in Celcius.
Couple of examples:
Send "Am+250" to get alerted when temperature rises above +25 Celcius 
Send "Al-050" to get alerted when temperature drops below - 5 Celcius
Send "Am-150" to get alerted when temperature rises above -15 Celcius
Send "Al+200" to get alerted when temperature drops below +20 Celcius
