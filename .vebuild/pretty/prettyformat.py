def prettyformat(command = "??", module="Unknown", description="Somebody please fix this!!", bCustom=True):
    if bCustom:
        pretty_command = "*[%s]" % (command[:5])
    else:
        pretty_command = " [%s]" % (command[:5])
    
    pretty_module =  "[%s]" % (module[:15])
    
    print " %-8s %-17s %s" % (pretty_command, pretty_module, description)

