import matplotlib.pyplot as plt
import numpy as np

x = np.arange(2.5, 12.5, 0.01)

plt.title("TestPLOT")
plt.plot(x, x**2, 'r--', label = 'y = x^2')
plt.plot(x, 2**x / 8, 'b-', label = 'y = 2^x / 8')

plt.xlabel("X Data")
plt.ylabel("Y Data")

plt.legend()

plt.grid(True)
plt.show()



# import matplotlib.pyplot as plt
# import numpy as np
#
# # x 좌표는 0부터 0.01씩 더해져 최대 20까지
# x = np.arange(0.0, 20.0, 0.01)
# # y = x^2 그래프
# plt.subplot(221)
# plt.plot(x, x ** 2, 'r--')
# # y = 2^x 그래프
# plt.subplot(222)
# plt.plot(x, 2 ** x, 'b-')
# # y = x
# plt.subplot(223)
# plt.plot( x**2, x*4, 'g')
# # y = 1/(x+1)
# plt.subplot(224)
# plt.plot(x, 1 / (x + 1))
#
# plt.show()


# import matplotlib.pyplot as plt
# import numpy as np
#
# x = np.arange(0.0, 5.0, 0.01)
#
# # y = x^2 그래프
# plt.plot(x, x**2, 'r--')
# # y = 2^x 그래프
# plt.plot(x, 2**x, 'b')
#
# plt.axis([0,6,0,40])
# plt.show()