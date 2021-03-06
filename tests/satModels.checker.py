def decodeModels(string):
    models = []

    model = set()
    for line in string.split("\n"):
        line = line.strip()
        if len(line) == 0: continue
        if line[0] == 'v':
            model = model | set(line[2:].split(" "))
        elif line[0] == 'c' and len(model) > 0:
            models.append(model)
            model = set()

    if len(model) > 0:
        models.append(model)
    return models

def checkModel(model):
    global input
    for clause in input.split("\n"):
        if clause and clause[0] != 'c' and clause[0] != 'p':
            sat = False
            for lit in clause.split(" "):
                if lit != '0' and lit in model:
                    sat = True
                    break
            if not sat:
                return False
    return True
        

def checker(actualOutput, actualError):
    global output
    
    if actualError:
        reportFailure(output, actualError)
        return

    if not actualOutput:
        reportFailure(output, "No output stream!")
        return
            
    models = decodeModels(actualOutput)
    if output.strip().lower() == "sat":
        if len(models) == 0:
            reportFailure("SATISFIABLE", "UNSATISFIABLE")
        else:
            for model in models:
                if not checkModel(model):
                    reportFailure("SATISFIABLE", "%s; wrong model: %s" % (models, model))
                    return
            reportSuccess("SATISFIABLE", models)
    elif len(models) > 0:
        reportFailure("UNSATISFIABLE", models)
    else:
        reportSuccess("UNSATISFIABLE", "UNSATISFIABLE")
