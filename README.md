<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="description" content="README file for Rotary Encoder and TFT Display with LittlevGL Interface Code">
  <style>
    body {
      font-family: Arial, sans-serif;
      line-height: 1.6;
      margin: 20px;
      color: #333;
    }
    h1 {
      color: #2c3e50;
    }
    h2 {
      color: #2980b9;
    }
    p {
      margin-bottom: 15px;
    }
    ul {
      margin-bottom: 15px;
    }
    code {
      background-color: #f4f4f4;
      padding: 5px;
      font-family: Consolas, monospace;
    }
    pre {
      background-color: #f4f4f4;
      padding: 15px;
      overflow-x: auto;
    }
    a {
      color: #3498db;
      text-decoration: none;
    }
  </style>
</head>
<body>

  <h1>Rotary Encoder & TFT Display with LittlevGL Interface - README</h1>

  <h2>1. Overview</h2>
  <p>
    This project utilizes a rotary encoder and a TFT touchscreen display, combined with the <a href="https://lvgl.io/">LittlevGL (lvgl)</a> graphics library to create an interactive user interface. The system allows users to navigate through a list of items, view sublists, and interact with the display using both touch and a rotary encoder.
  </p>

  <h2>2. Purpose</h2>
  <p>
    The primary goal of this code is to implement a simple GUI (Graphical User Interface) system on a TFT screen, where users can scroll through a list of items using a rotary encoder and select them via a button press. Additionally, the GUI supports sublist navigation, making it a useful framework for embedded system applications that require a minimal yet functional interface.
  </p>

  <h2>3. How It Works</h2>
  <p>
    The core functionality of the project includes:
  </p>
  <ul>
    <li><b>Rotary Encoder:</b> The rotary encoder is used to scroll through items in the list and sublist. The direction of rotation determines whether the counter increases or decreases, thus selecting different items.</li>
    <li><b>Button Press:</b> The button on the rotary encoder is used to "select" the currently highlighted item. If the user is in the main list, they can navigate to a sublist, or they can return from the sublist to the main list.</li>
    <li><b>LittlevGL (lvgl) GUI:</b> The LittlevGL library manages the graphical elements on the screen, such as the list items and their styles. It also provides event handling for click actions and updates the display based on user input.</li>
  </ul>

  <h2>4. Applications</h2>
  <p>
    This code can be applied to a variety of projects, including:
  </p>
  <ul>
    <li><b>Embedded System Interfaces:</b> This project serves as an interactive interface for embedded systems that require minimal GUI, such as industrial machines or home automation systems.</li>
    <li><b>Menu Navigation:</b> Useful in projects where menu-based navigation is required, such as settings screens, media players, or IoT devices.</li>
    <li><b>Product Prototypes:</b> Developers can use this code as a base for rapid prototyping of embedded systems that need touch and rotary controls.</li>
    <li><b>Control Panels:</b> Can be integrated into custom control panels for electronics, robotics, or automation projects.</li>
  </ul>

  <h2>5. Key Features</h2>
  <ul>
    <li><b>Interactive User Interface:</b> Allows users to scroll through lists, select items, and navigate submenus using a combination of touch and rotary encoder input.</li>
    <li><b>Rotary Encoder Support:</b> The rotary encoder provides an intuitive way to interact with the system without a complex touch interface.</li>
    <li><b>Customizable Styles:</b> LittlevGL allows developers to customize item styles, making the interface visually appealing and easy to understand.</li>
    <li><b>Touchscreen Support:</b> In addition to the rotary encoder, a TFT touchscreen is integrated, making the interface more versatile.</li>
  </ul>

  <h2>6. Libraries Used</h2>
  <ul>
    <li><b>LittlevGL (lvgl):</b> A powerful and open-source graphics library for embedded devices that manages the GUI, event handling, and rendering.</li>
    <li><b>Adafruit TFT and Touchscreen Libraries:</b> Used to interface with the TFT display and touch input.</li>
    <li><b>Arduino Core:</b> The project is built using Arduino IDE, making it accessible for a wide range of developers.</li>
  </ul>

  <h2>7. Code Breakdown</h2>
  <p>
    The code consists of several key functions that drive the project:
  </p>
  <ul>
    <li><b>setup():</b> Initializes the rotary encoder, button, TFT display, and LittlevGL environment.</li>
    <li><b>loop():</b> Continuously handles user inputs (encoder rotation and button presses) and updates the GUI accordingly.</li>
    <li><b>handle_encoder_list() & handle_encoder_sublist():</b> Manage the encoder input for navigating through the main list and sublist, respectively.</li>
    <li><b>handle_button_press():</b> Handles the button press to select items from the list or sublist.</li>
    <li><b>lv_create_sublist() & lv_remove_sublist():</b> Manage the creation and removal of sublists from the display.</li>
  </ul>

  <h2>8. Potential Future Work</h2>
  <p>
    There are several areas for improvement and future development:
  </p>
  <ul>
    <li><b>Dynamic List Items:</b> Currently, the list and sublist items are static. Future improvements could involve dynamically populating the list based on external inputs, such as sensor data or external devices.</li>
    <li><b>Additional Input Methods:</b> While the rotary encoder and touch input work well, additional methods such as voice control or wireless control (Bluetooth/Wi-Fi) could be added.</li>
    <li><b>Enhanced Visuals:</b> Adding animations or more complex styles to the UI could make it more user-friendly and visually appealing.</li>
    <li><b>Multi-Layer Menus:</b> Implementing deeper menu structures (more than two levels) could be useful for more complex applications.</li>
    <li><b>Integration with IoT:</b> The system could be integrated with IoT platforms to control connected devices and display real-time data, making it more versatile for smart home or industrial applications.</li>
  </ul>

</body>
</html>
