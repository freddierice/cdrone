package main

import (
	"fmt"
	"os"
	"time"

	"./base"
)

func startBase() {
	baseConfig, err := base.ParseConfig("gobase.conf")
	if err != nil {
		fmt.Fprintf(os.Stderr, "could not parse gobase.conf: %v", err)
		os.Exit(1)
	}

	base, err := base.New(*baseConfig)
	if err != nil {
		fmt.Fprintf(os.Stderr, "could not create base: %v", err)
		os.Exit(1)
	}

	// wait a bit
	time.Sleep(10 * time.Second)

	fmt.Println("shutting down.")
	base.Close()
}

func main() {
	/*
		// create application
		app := widgets.NewQApplication(len(os.Args), os.Args)

		// main window
		window := widgets.NewQMainWindow(nil, 0)
		window.SetWindowTitle("gobase")
		window.SetMinimumSize2(200, 200)

		// Start the app
		app.Exec()
	*/

	startBase()
}

/*
	// Create application
	app := widgets.NewQApplication(len(os.Args), os.Args)

	// Create main window
	window := widgets.NewQMainWindow(nil, 0)
	window.SetWindowTitle("Hello World Example")
	window.SetMinimumSize2(200, 200)

	// Create main layout
	layout := widgets.NewQVBoxLayout()

	// Create main widget and set the layout
	mainWidget := widgets.NewQWidget(nil, 0)
	mainWidget.SetLayout(layout)

	// Create a line edit and add it to the layout
	input := widgets.NewQLineEdit(nil)
	input.SetPlaceholderText("1. write something")
	layout.AddWidget(input, 0, 0)

	// Create a button and add it to the layout
	button := widgets.NewQPushButton2("2. click me", nil)
	layout.AddWidget(button, 0, 0)

	// Connect event for button
	button.ConnectClicked(func(checked bool) {
		widgets.QMessageBox_Information(nil, "OK", input.Text(),
		widgets.QMessageBox__Ok, widgets.QMessageBox__Ok)
	})

	// Set main widget as the central widget of the window
	window.SetCentralWidget(mainWidget)

	// Show the window
	window.Show()

	// Execute app
	app.Exec()
*/
