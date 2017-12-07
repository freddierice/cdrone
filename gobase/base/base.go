package base

import (
	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"sync"
	"time"

	"../proto"
	protobuf "github.com/golang/protobuf/proto"
)

// Base holds a connection to a base.
type Base struct {
	conn        net.Conn
	stop        bool
	messageChan chan protobuf.Message
	wg          *sync.WaitGroup
}

// Config holds configurations for a Base.
type Config struct {
	Host                string `json:"host"`
	Port                string `json:"port"`
	Certificate         []byte `json:"-"`
	Key                 []byte `json:"-"`
	CA                  []byte `json:"-"`
	certificateFilename string `json:"certificate"`
	keyFilename         string `json:"key"`
	caFilename          string `json:"ca"`
	SSL                 bool   `json:"ssl"`
}

// ParseConfig parses a json configuration file to a BaseConfig.
func ParseConfig(filename string) (*Config, error) {
	buf, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	config := &Config{}
	if err := json.Unmarshal(buf, config); err != nil {
		return nil, err
	}

	if config.SSL {
		buf, err = ioutil.ReadFile(config.certificateFilename)
		if err != nil {
			return nil, err
		}
		config.Certificate = buf

		buf, err = ioutil.ReadFile(config.keyFilename)
		if err != nil {
			return nil, err
		}
		config.Key = buf

		buf, err = ioutil.ReadFile(config.caFilename)
		if err != nil {
			return nil, err
		}
		config.CA = buf
	}

	return config, nil
}

// New uses baseConfig to create a Base handle to a drone.
func New(config Config) (*Base, error) {
	var conn net.Conn
	var err error
	hostname := config.Host + ":" + config.Port

	if config.SSL {
		roots := x509.NewCertPool()
		ok := roots.AppendCertsFromPEM(config.CA)
		if !ok {
			return nil, fmt.Errorf("")
		}
		tlsConfig := &tls.Config{
			RootCAs: roots,
		}

		conn, err = tls.Dial("tcp", hostname, tlsConfig)
		if err != nil {
			return nil, err
		}
	} else {
		conn, err = net.DialTimeout("tcp", hostname, 3*time.Second)
		if err != nil {
			return nil, err
		}
	}

	// initialize the base for reference
	base := &Base{
		conn: conn,
		stop: false,
	}

	// start the heartbeats for the life of this base.
	messageChan := make(chan protobuf.Message, 10)
	wg := &sync.WaitGroup{}
	wg.Add(2)
	defer func() {
		go func() {
			defer wg.Done()
			for pb := range messageChan {
				if err := base.SendMessage(pb); err != nil {
					fmt.Printf("error sending message: %v\n", err)
				}
			}
		}()
		go func() {
			defer wg.Done()
			defer close(messageChan)
			for !base.stop {
				time.Sleep(time.Second)
				messageChan <- &proto.Update{
					Mode: proto.UpdateMode_NO_MODE_CHANGE,
				}
			}
		}()
	}()

	base.messageChan = messageChan
	base.wg = wg

	return base, nil
}

func writeFull(w io.Writer, buf []byte) error {
	n := 0
	for n != len(buf) {
		n, err := w.Write(buf)
		if err != nil {
			return err
		}
		buf = buf[n:]
	}

	return nil
}

// SendMessage sends a protobuf message to the drone.
func (base *Base) SendMessage(pb protobuf.Message) error {
	pbLen := protobuf.Size(pb)
	buf, err := protobuf.Marshal(pb)
	if err != nil {
		return err
	}

	lenBuf := protobuf.EncodeVarint(uint64(pbLen))
	if err := writeFull(base.conn, lenBuf); err != nil {
		return err
	}
	if err := writeFull(base.conn, buf); err != nil {
		return err
	}

	return nil
}

// Close closes a connection to the drone.
func (base *Base) Close() {
	base.stop = true
	base.wg.Wait()
	if base.conn != nil {
		base.conn.Close()
		base.conn = nil
	}
}
