package base

import (
	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"time"
)

// Base holds a connection to a base.
type Base struct {
	conn net.Conn
	stop chan bool
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

	// start the heartbeats for the life of this base.
	stop := make(chan bool, 1)
	//messageChan := make(chan
	go func() {
		for stop != nil {
			time.Sleep(time.Second)
		}
	}()

	return &Base{
		conn: conn,
		stop: stop,
	}, nil
}

// Close closes a connection to the drone.
func (base *Base) Close() {
	if base.stop != nil {
		close(base.stop)
	}
	// TODO: add 'join' channels.
	if base.conn != nil {
		base.conn.Close()
		base.conn = nil
	}
}
