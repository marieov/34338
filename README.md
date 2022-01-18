# Arduino mailbox project

In this project we have made a mail-detection system using Arduino and ESP32/ESP8266. A digital sensor (tilt sensor) detects when the flip on the mailbox is opened and an analogue sensor (phototransistor) detects when there is light coming in. An email is then sent to the user over WiFi. To collect the mail the user opens a web page and clicks a button, this starts a timer that prevents the email service from sending email for the next ten minutes. 

The motivation behind the project is to not check the mailbox unless there is mail in it, and to remember to collect the mail. The system is powered by a power bank. This makes the system more sustainable as no battery is needed and it can be recharged.

Cource 34448 Telecommunication programming projects with Arudino at DTU January 2022.
