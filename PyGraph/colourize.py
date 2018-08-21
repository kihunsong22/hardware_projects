# Colourise - colours text in shell. Returns plain if colour doesn't exist.
def colourise(colour, text):
    if colour == "black":
        return "\033[1;30m" + str(text) + "\033[1;m"
    if colour == "red":
        return "\033[1;31m" + str(text) + "\033[1;m"
    if colour == "green":
        return "\033[1;32m" + str(text) + "\033[1;m"
    if colour == "yellow":
        return "\033[1;33m" + str(text) + "\033[1;m"
    if colour == "blue":
        return "\033[1;34m" + str(text) + "\033[1;m"
    if colour == "magenta":
        return "\033[1;35m" + str(text) + "\033[1;m"
    if colour == "cyan":
        return "\033[1;36m" + str(text) + "\033[1;m"
    if colour == "gray":
        return "\033[1;37m" + str(text) + "\033[1;m"
    return str(text)

# Highlight - highlights text in shell. Returns plain if colour doesn't exist.
def highlight(colour, text):
    if colour == "black":
        return "\033[1;40m" + str(text) + "\033[1;m"
    if colour == "red":
        return "\033[1;41m" + str(text) + "\033[1;m"
    if colour == "green":
        return "\033[1;42m" + str(text) + "\033[1;m"
    if colour == "yellow":
        return "\033[1;43m" + str(text) + "\033[1;m"
    if colour == "blue":
        return "\033[1;44m" + str(text) + "\033[1;m"
    if colour == "magenta":
        return "\033[1;45m" + str(text) + "\033[1;m"
    if colour == "cyan":
        return "\033[1;46m" + str(text) + "\033[1;m"
    if colour == "gray":
        return "\033[1;47m" + str(text) + "\033[1;m"
    return str(text)

# # Example usage:
# print(colourise("black", "Black"))
# print(colourise("red", "Red"))
# print(colourise("green", "Green"))
# print(colourise("yellow", "Yellow"))
# print(colourise("blue", "Blue"))
# print(colourise("magenta", "Magenta"))
# print(colourise("cyan", "Cyan"))
# print(colourise("gray", "Gray"))
# print(highlight("black", "Highlight: black"))
# print(highlight("red", "Highlight: red"))
# print(highlight("green", "Highlight: green"))
# print(highlight("yellow", "Highlight: yellow"))
# print(highlight("blue", "Highlight: blue"))
# print(highlight("magenta", "Highlight: magenta"))
# print(highlight("cyan", "Highlight: cyan"))
# print(highlight("gray", "Highlight: gray"))
#
# # Example usage of colourise() + highlight()
# text = "Blue on red is difficult to read because the wavelengths are \
# refracted onto different areas of the eye."
#
# print(highlight("blue", (colourise("red", text))))
# print(highlight("red", (colourise("blue", text))))