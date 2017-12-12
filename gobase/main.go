package main

import (
	"fmt"
	"os"

	"./base"
	termbox "github.com/nsf/termbox-go"
)

const DefaultHeight = 0.35

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
	x := float64(0.0)
	y := float64(0.0)
	z := float64(0.35)

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
				z += 0.05
				b.SetVelocity(x, y, z)
			} else if ev.Ch == 'a' {
				b.SetVelocity(x, y, z)
			} else if ev.Ch == 's' {
				z -= 0.05
				b.SetVelocity(x, y, z)
			} else if ev.Ch == 'd' {
				x = 0.1
				b.SetVelocity(x, y, z)
			} else if ev.Ch == 'h' {
				x = 0
				y = 0
				z = DefaultHeight
				b.SetVelocity(x, y, z)
			} else if ev.Ch == 'i' {
				y = 0.1
				b.SetVelocity(x, y, z)
			} else if ev.Ch == 'j' {
				x = -0.1
				b.SetVelocity(x, y, z)
			} else if ev.Ch == 'k' {
				y = -0.1
				b.SetVelocity(x, y, z)
			} else if ev.Ch == 'l' {
				x = 0.1
				b.SetVelocity(x, y, z)
			}
		}
	}

}
