package main

import (
	"encoding/json"
	"fmt"
	"html/template"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"strings"
)

var headerTmpl, cppTmpl *template.Template

// HeaderContext holds the TypePairs
type HeaderContext struct {
	Name      string            `json:"name"`
	TypePairs map[string]string `json:"typePairs"`
}

// CppContext holds context for Cpp template execution.
type CppContext struct {
	Name  string
	Types []HeaderContext
}

func main() {
	if len(os.Args) != 3 {
		fmt.Printf("usage: %v <conf> <out directory>\n", os.Args[0])
		return
	}

	createTemplates()

	confFilename := os.Args[1]
	outDir, err := filepath.Abs(os.Args[2] + "/")
	if err != nil {
		fmt.Printf("bad path\n")
		os.Exit(1)
	}

	confBytes, err := ioutil.ReadFile(confFilename)
	if err != nil {
		fmt.Printf("error parsing generator.conf: %v\n", err)
		os.Exit(1)
	}

	contextArray := []HeaderContext{}
	err = json.Unmarshal(confBytes, &contextArray)

	for _, ctx := range contextArray {
		err = writePair(outDir, ctx)
		if err != nil {
			fmt.Printf("error: %v\n", err)
			os.Exit(1)
		}
	}

	cppContext := CppContext{
		Name:  "generated",
		Types: contextArray,
	}
	err = writeCpp(outDir, cppContext)
	if err != nil {
		fmt.Printf("error: %v\n", err)
		os.Exit(1)
	}
}

func writeCpp(dir string, cppContext CppContext) error {

	cppFile, err := os.Create(path.Join(dir, cppContext.Name+".cpp"))
	if err != nil {
		return err
	}
	defer cppFile.Close()

	cppTmpl.Execute(cppFile, cppContext)
	return nil
}

func writePair(dir string, headerContext HeaderContext) error {

	headerFile, err := os.Create(path.Join(dir, headerContext.Name+".h"))
	if err != nil {
		return err
	}
	defer headerFile.Close()

	headerTmpl.Execute(headerFile, headerContext)
	return nil
}

func createTemplates() {

	var err error

	funcMap := template.FuncMap{
		"ToUpper": strings.ToUpper,
		"Title":   strings.Title,
	}

	headerCode := `#ifndef __LOGGER_{{.Name | ToUpper}}_H__ 
#define __LOGGER_{{.Name | ToUpper}}_H__ 
#include "logging/logging.h"

namespace logging {
		typedef struct {{.Name}}_struct { {{range $name, $type := $.TypePairs}}
			{{$type}} {{$name}};{{end}}
		} __attribute__((packed)) {{.Name}}_t;

	class {{.Name | Title}}Variable : public Variable {
		virtual std::string name() {
			return "{{.Name}}";
		}
		virtual unsigned int size() {
			return {{range $name, $type := $.TypePairs}} sizeof({{$type}}) + {{end}} 0;
		}
		virtual std::vector<std::string> names() {
			return {
			{{range $name, $type := $.TypePairs}} "{{$name}}",
			{{end}}
			};
		}
		virtual std::vector<std::string> types() {
			return {
			{{range $name, $type := $.TypePairs}} "{{$name}}",
			{{end}}
			};
		}
	};
	extern {{.Name | Title}}Variable {{.Name}}_variable;
}
#endif
`

	cppCode := `{{range $i, $x := .Types}}#include "logging/{{$x.Name}}.h"
{{end}}

namespace logging {
	{{range $i, $x := .Types}}{{$x.Name | Title}}Variable {{.Name}}_variable;
	{{end}}
}
`

	headerTmpl, err = template.New("header").Funcs(funcMap).Parse(headerCode)
	if err != nil {
		fmt.Printf("%v\n", err)
		os.Exit(1)
	}

	cppTmpl, err = template.New("cpp").Funcs(funcMap).Parse(cppCode)
	if err != nil {
		fmt.Printf("%v\n", err)
		os.Exit(1)
	}
}
