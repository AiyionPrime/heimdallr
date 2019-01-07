#!/usr/bin/env bash
_heimdallr_completitions()
{
	singles="-h|-V|-u|-s"
	tuples="-p"

	defaultports="4567 22222"
	

	if [[ "${COMP_WORDS[1]}" =~ ^($singles)$ ]]; then
		return
	fi

	if [ "${#COMP_WORDS[@]}" -gt "3" ]; then
		return
	fi


	if [ "${COMP_WORDS[1]}" = "-p" ]; then
		COMPREPLY=($(compgen -W "$defaultports" -- "${COMP_WORDS[2]}"))
	else
		COMPREPLY=($(compgen -W "-h -p -s -u -V" "\"${COMP_WORDS[1]}\""))
	fi

}
complete -F _heimdallr_completitions heimdallr
