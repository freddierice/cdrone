package main

import (
	"fmt"
	"os"
	"time"

	"./base"
)

func startGUI() {
}

func readKeys() {
}

func main() {
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

	startGUI()
	readKeys()

	time.Sleep(20 * time.Second)

	fmt.Println("shutting down.")
	base.Close()
}
