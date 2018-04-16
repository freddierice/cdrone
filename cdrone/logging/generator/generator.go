package main

import (
	"fmt"
	"html/template"
	"os"
	"strings"
)

// Context holds the TypePairs
type Context struct {
	Name      string
	TypePairs []TypePair
}

// TypePair holds a (typename, varname) pair.
type TypePair struct {
	TypeName string
	VarName  string
}

func main() {
	if len(os.Args) < 4 || len(os.Args)%2 != 0 {
		fmt.Printf("usage: %v <name> <typename1> <varname1> [(<typename2> <varname2>) ...]\n", os.Args[0])
		return
	}

	typePairs := make([]TypePair, (len(os.Args)-2)/2)
	for i := 0; i < len(typePairs); i++ {
		typePairs[i] = TypePair{
			TypeName: os.Args[2+i*2],
			VarName:  os.Args[3+i*2],
		}
	}

	funcMap := template.FuncMap{
		"ToUpper": strings.ToUpper,
		"Title":   strings.Title,
	}

	context := Context{
		Name:      os.Args[1],
		TypePairs: typePairs,
	}

	tmplCode := `#ifndef __LOGGER_{{.Name | ToUpper}}_H__ 
#define __LOGGER_{{.Name | ToUpper}}_H__ 
#include "logging.h"

namespace logging {
		typedef struct {{.Name}}_struct { {{range $i, $x := $.TypePairs}}
			{{$x.TypeName}} {{$x.VarName}};{{end}}
		} __attribute__((packed)) {{.Name}}_t;

	class {{.Name | Title}}Variable : public Variable {
		virtual std::string name() {
			return "{{.Name}}";
		};
		virtual unsigned int size() {
			return {{range $i, $x := $.TypePairs}} sizeof({{$x.TypeName}}) + {{end}} 0;
		};
		virtual std::vector<std::string> names() {
			return {
			{{range $i, $x := $.TypePairs}} "{{$x.VarName}}",
			{{end}}
			};
		};
		virtual std::vector<std::string> types() {
			return {
			{{range $i, $x := $.TypePairs}} "{{$x.TypeName}}",
			{{end}}
			};
		};
	};
	{{.Name | Title}}Variable {{.Name}}_variable;
}
#endif
`

	tmpl, err := template.New("TypePairs").Funcs(funcMap).Parse(tmplCode)
	if err != nil {
		fmt.Printf("%v\n", err)
		return
	}
	tmpl.Execute(os.Stdout, context)
}
