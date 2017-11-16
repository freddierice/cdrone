package main

import (
	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"time"
)

// Base holds a connection to a base.
type Base struct {
	conn io.ReadWriter
	stop chan bool
}

// BaseConfig holds configurations for a Base.
type BaseConfig struct {
	Host                string
	Port                string
	Certificate         []byte
	Key                 []byte
	CA                  []byte
	certificateFilename string
	keyFilename         string
	caFilename          string
}

// ParseBaseConfig parses a json configuration file to a BaseConfig.
func ParseBaseConfig(filename string) (*BaseConfig, error) {
	buf, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	baseConfig := &BaseConfig{}
	if err := json.Unmarshal(buf, baseConfig); err != nil {
		return nil, err
	}

	buf, err = ioutil.ReadFile(baseConfig.certificateFilename)
	if err != nil {
		return nil, err
	}
	baseConfig.Certificate = buf

	buf, err = ioutil.ReadFile(baseConfig.keyFilename)
	if err != nil {
		return nil, err
	}
	baseConfig.Key = buf

	buf, err = ioutil.ReadFile(baseConfig.caFilename)
	if err != nil {
		return nil, err
	}
	baseConfig.CA = buf

	return baseConfig, nil
}

// ConnectBase uses baseConfig to create a Base handle to a drone.
func ConnectBase(baseConfig BaseConfig) (*Base, error) {

	roots := x509.NewCertPool()
	ok := roots.AppendCertsFromPEM(baseConfig.CA)
	if !ok {
		return nil, fmt.Errorf("")
	}
	tlsConfig := &tls.Config{
		RootCAs: roots,
	}

	hostname := baseConfig.Host + ":" + baseConfig.Port
	conn, err := tls.Dial("tcp", hostname, tlsConfig)
	if err != nil {
		return nil, err
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
