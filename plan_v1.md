I have an esp 32 board connected to a waveshare 7.5in black and white e-paper display.

This project will be to build a simple clock and wheather display using the esp32 and the e-paper display.

The clock will display the current time and date using book quotes that can be found in this repository: https://github.com/ambercaravalho/open-author-clock

Specifically in the `data.json` file.

Times are represented as json objects like so:

```json
{
    "time": "00:00",
    "timeString": "midnight",
    "quote": "And almost exactly at midnight, the Count’s patience was rewarded. For in accordance with the instructions he’d written to Richard, every telephone on the first floor of the Metropol began to ring.",
    "title": "A Gentleman in Moscow",
    "author": "Amor Towles"
}
```

there are multiple quotes for each time, so the clock will randomly select one of the quotes for the current time to display.

The current weather will be displayed using the OpenWeatherMap API. The weather will be updated every 30 minutes. The api key can be found in the `.env`

The clock should be the main focus of the display, with the weather information displayed in a smaller font in the top right corner of the display. The weather information should include the current temperature, weather condition, and an icon representing the weather condition.

The clock should update every minute to display the current time and date. The quote should use a random font from a selection of fonts that are stored on the esp32.

Connecting the esp32 to the wifi network should follow these steps.

1. Power on the esp32.
2. Display a url or ip address on the e-paper display that the user can navigate to on their phone or computer.
3. The user navigates to the url or ip address and is presented with a simple web page that allows them to enter their wifi network name and password.
4. The user submits the form and the esp32 connects to the wifi network using the provided credentials.
5. Once connected, the esp32 displays a confirmation message on the e-paper display and begins fetching the current time and weather information to display on the clock.

The connection process should be secure and the wifi credentials should not be stored in plain text on the esp32. The esp32 should use a secure method of storing the wifi credentials, such as encryption or hashing.

The code for this project should be written in C++ using the Arduino framework. The code should be well organized and commented to make it easy to understand and maintain. The code should also be modular, with separate functions for fetching the time and weather information, updating the display, and handling the wifi connection process.

The project should be designed to be easily extendable, allowing for additional features to be added in the future, such as displaying news headlines or calendar events.