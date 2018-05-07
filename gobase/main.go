package main

import (
	"fmt"
	"os"
	"time"

	"./base"
	"./proto"
	termbox "github.com/nsf/termbox-go"
)

// DefaultHeight holds the initial height of the drone in meters.
const DefaultHeight = 0.31

// TiltStep is the step between tilts.
const TiltStep = uint32(5)

// VelocityStep holds the step size for the velocity.
const VelocityStep = 0.2

func printAt(x, y int, s string) {
	for _, r := range s {
		termbox.SetCell(x, y, r, termbox.ColorWhite, termbox.ColorBlack)
		x++
	}
}

func refresh(b *base.Base) {
	termbox.Clear(termbox.ColorWhite, termbox.ColorBlack)

	// print out the instructions
	printAt(0, 0, "INSTRUCTIONS")
	printAt(0, 1, "; - arm")
	printAt(0, 2, "[spacebar] - disarm")
	printAt(0, 3, "t - takeoff")
	printAt(0, 4, "a - yaw left")
	printAt(0, 5, "d - yaw right")
	printAt(0, 6, "w - up")
	printAt(0, 7, "s - down")
	printAt(0, 8, "j - left")
	printAt(0, 9, "l - right")
	printAt(0, 10, "i - forward")
	printAt(0, 11, "k - backward")
	printAt(0, 12, "h - hover")
	printAt(0, 13, "r - reset the image for position mode")
	printAt(0, 14, "p - toggle position mode")
	printAt(0, 15, "1 - lower angle of tilt")
	printAt(0, 16, "2 - increase angle of tilt")

	// increment 40
	printAt(40, 0, "VALUES")
	printAt(40, 1, fmt.Sprintf("Battery: %v", b.Drone.Battery))
	printAt(40, 2, fmt.Sprintf("Mode: %v", b.Drone.Mode))
	printAt(40, 3, fmt.Sprintf("VelocityX: %v", b.Drone.VelocityX))
	printAt(40, 4, fmt.Sprintf("VelocityY: %v", b.Drone.VelocityY))
	printAt(40, 5, fmt.Sprintf("PositionX: %v", b.Drone.PositionX))
	printAt(40, 6, fmt.Sprintf("PositionY: %v", b.Drone.PositionY))
	printAt(40, 7, fmt.Sprintf("PositionZ: %v", b.Drone.PositionZ))
	printAt(40, 8, fmt.Sprintf("AngRollVel: %v", b.Drone.SkylineAngRollVel))
	printAt(40, 9, fmt.Sprintf("AngPitchVel: %v", b.Drone.SkylineAngPitchVel))
	printAt(40, 10, fmt.Sprintf("Throttle: %v", b.Drone.Throttle))
	printAt(40, 11, fmt.Sprintf("Roll: %v", b.Drone.Roll))
	printAt(40, 12, fmt.Sprintf("Pitch: %v", b.Drone.Pitch))
	printAt(40, 13, fmt.Sprintf("Yaw: %v", b.Drone.Yaw))

	termbox.Flush()
}

func doInput() error {
	err := termbox.Init()
	if err != nil {
		return err
	}
	defer termbox.Close()

	termbox.SetInputMode(termbox.InputAlt)

	// initialize the base
	baseConfig, err := base.ParseConfig("gobase.conf")
	if err != nil {
		return fmt.Errorf("could not parse gobase.conf: %v", err)
	}

	b, err := base.New(*baseConfig)
	if err != nil {
		return fmt.Errorf("could not create base: %v", err)
	}
	defer b.Close()

	// refresh screen
	refresh(b)

	done := false
	go func() {
		defer func() { done = true }()

		// initialize some variables
		vx := float64(0.0)
		vy := float64(0.0)
		x := float64(0.0)
		y := float64(0.0)
		z := float64(DefaultHeight)
		tilt := uint32(150)

		for {
			switch ev := termbox.PollEvent(); ev.Type {
			case termbox.EventKey:
				if ev.Ch == 'q' {
					fmt.Println("shutting down")
					done = true
					return
				} else if ev.Ch == ';' {
					b.Arm()
				} else if ev.Key == termbox.KeySpace {
					b.Disarm()
				} else if ev.Ch == 't' {
					b.Takeoff()
				} else if ev.Ch == 'p' {
					b.Position()
				} else if ev.Ch == 'v' {
					b.Velocity()
				} else if ev.Ch == 'r' {
					b.ResetPosition()
				} else if ev.Ch == '1' {
					tilt -= TiltStep
					if tilt < 80 {
						tilt = 80
					}
					b.SetTilt(tilt)
				} else if ev.Ch == '2' {
					tilt += TiltStep
					if tilt > 240 {
						tilt = 240
					}
					b.SetTilt(tilt)
				}
				if b.Drone.Mode == proto.Mode_VELOCITY {
					if ev.Ch == 'w' {
						z += 0.05
						b.SetVelocity(vx, vy, z)
					} else if ev.Ch == 'a' {
						b.SetVelocity(vx, vy, z)
					} else if ev.Ch == 's' {
						z -= 0.05
						b.SetVelocity(vx, vy, z)
					} else if ev.Ch == 'd' {
						vx = VelocityStep
						b.SetVelocity(vx, vy, z)
					} else if ev.Ch == 'h' {
						vx = 0
						vy = 0
						b.SetVelocity(vx, vy, z)
					} else if ev.Ch == 'i' {
						vy = VelocityStep
						b.SetVelocity(vx, vy, z)
					} else if ev.Ch == 'j' {
						vx = -VelocityStep
						b.SetVelocity(vx, vy, z)
					} else if ev.Ch == 'k' {
						vy = -VelocityStep
						b.SetVelocity(vx, vy, z)
					} else if ev.Ch == 'l' {
						vx = 0.1
						b.SetVelocity(vx, vy, z)
					}
				} else if b.Drone.Mode == proto.Mode_POSITION {
					if ev.Ch == 'w' {
						z += 0.05
						b.SetPosition(x, y, z)
					} else if ev.Ch == 'a' {
						b.SetPosition(x, y, z)
					} else if ev.Ch == 's' {
						z -= 0.05
						b.SetPosition(x, y, z)
					} else if ev.Ch == 'd' {
						x += 0.1
						b.SetPosition(x, y, z)
					} else if ev.Ch == 'i' {
						y += 0.1
						b.SetPosition(x, y, z)
					} else if ev.Ch == 'j' {
						x -= 0.1
						b.SetPosition(x, y, z)
					} else if ev.Ch == 'k' {
						y -= 0.1
						b.SetPosition(x, y, z)
					} else if ev.Ch == 'l' {
						x += 0.1
						b.SetPosition(x, y, z)
					}
				}
			}
		}
	}()

	for !done {
		refresh(b)
		time.Sleep(75 * time.Millisecond)
	}

	return nil
}

func main() {
	if err := doInput(); err != nil {
		fmt.Fprintf(os.Stderr, "err doing input: %v\n", err)
		os.Exit(1)
	}
}
