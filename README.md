# wot-challenge3

## Camera onn-off mqtt (arduino)
Hierbij ga je IR signalen versturen naar een ir sensor. Deze sensor gaat aan de hand van een library deze opvangen.
Elke camera heeft een bepaalde frequentue van signalen (deze code staat op 3). De signalen moeten met juiste frequentie verstuurd worden tot als lampje blijft bvranden en camera "broke" is.

## Camera webserver (arduino)
Deze code is voor een esp32 cam in te stellen en deze een eigen ip adres toe te wijzen op het netwerk waardoor je deze kan streamen hierop.

## Challenge 3 (op PI)
In deze map ga je de code vinden voor flask server te runnen in python (main.py). Deze gaat eerst code vereisen om daarna te redirecten naar dashboard.html waar je de 4 cmera schermen ziet.
Met mqtt moet je deze vier schermen uitschakelen.
