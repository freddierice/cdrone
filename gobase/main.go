package main

import (
	"fmt"
	"os"

	"./base"
	termbox "github.com/nsf/termbox-go"
)

func main() {
	err := termbox.Init()
	if err != nil {
		panic(err)
	}
	defer termbox.Close()

	termbox.SetInputMode(termbox.InputAlt)
	termbox.Clear(termbox.ColorDefault, termbox.ColorDefault)
	termbox.Flush()

	// print out the instructions
	fmt.Println("INSTRUCTIONS")
	fmt.Println("; - arm")
	fmt.Println("[spacebar] - disarm")
	fmt.Println("t - takeoff")
	fmt.Println("a - yaw left")
	fmt.Println("d - yaw right")
	fmt.Println("w - up")
	fmt.Println("s - down")
	fmt.Println("j - left")
	fmt.Println("l - right")
	fmt.Println("i - forward")
	fmt.Println("k - backward")
	fmt.Println("h - hover")
	fmt.Println("r - reset the image used for position hold")
	fmt.Println("p - toggle position mode")

	// initialize the base
	baseConfig, err := base.ParseConfig("gobase.conf")
	if err != nil {
		fmt.Fprintf(os.Stderr, "could not parse gobase.conf: %v", err)
		os.Exit(1)
	}

	b, err := base.New(*baseConfig)
	if err != nil {
		fmt.Fprintf(os.Stderr, "could not create base: %v", err)
		os.Exit(1)
	}
	defer b.Close()

	// initialize some variables
	roll := uint32(1500)
	pitch := uint32(1500)
	yaw := uint32(1500)
	throttle := uint32(1000)

	// loop over the base
Done:
	for {
		switch ev := termbox.PollEvent(); ev.Type {
		case termbox.EventKey:
			if ev.Ch == 'q' {
				fmt.Println("shutting down")
				break Done
			} else if ev.Ch == ';' {
				b.Arm()
			} else if ev.Key == termbox.KeySpace {
				b.Disarm()
			} else if ev.Ch == 't' {
				b.Takeoff()
			} else if ev.Ch == 'p' {
				b.Position()
			} else if ev.Ch == 'r' {
				b.ResetPosition()
			} else if ev.Ch == 'w' {
				pitch = 1400
				b.SetVelocity(roll, pitch, yaw, throttle)
			} else if ev.Ch == 'a' {
				roll = 1600
				b.SetVelocity(roll, pitch, yaw, throttle)
			} else if ev.Ch == 's' {
				pitch = 1600
				b.SetVelocity(roll, pitch, yaw, throttle)
			} else if ev.Ch == 'd' {
				roll = 1400
				b.SetVelocity(roll, pitch, yaw, throttle)
			} else if ev.Ch == 'h' {
				roll = 1500
				pitch = 1500
				yaw = 1500
				b.SetVelocity(roll, pitch, yaw, throttle)
			} else if ev.Ch == 'i' {
			} else if ev.Ch == 'j' {
			} else if ev.Ch == 'k' {
			} else if ev.Ch == 'l' {

			}
		}
	}

}
