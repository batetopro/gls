import matplotlib.pyplot as plt
import matplotlib.lines as mlines


# Moves legend
conflicts = mlines.Line2D([], [], color='red', label='Conflicts')
guidance = mlines.Line2D([], [], color='blue', label='Guidance')
score = mlines.Line2D([], [], color='green', linestyle="--", label='10 * Score')

plt.legend(handles=[conflicts, guidance, score])
plt.show()

epochs = mlines.Line2D([], [], color='black', linestyle="--", label='Epochs')
best_improvement = mlines.Line2D([], [], color='black', marker="o", linestyle='None', label='Best improvement')

plt.legend(handles=[epochs, best_improvement])
plt.show()

# Performance legend
clean = mlines.Line2D([], [], color='blue', label='Clean GLS')
aspirations = mlines.Line2D([], [], color='blue', linestyle="--", label='Aspirations')
plt.legend(handles=[clean, aspirations])
plt.show()


clean = mlines.Line2D([], [], color='red', label='Keep penalties')
aspirations = mlines.Line2D([], [], color='red', linestyle="--", label='Keep penalties and aspirations')
plt.legend(handles=[clean, aspirations])
plt.show()


clean = mlines.Line2D([], [], color='black', label='Dynamic lambda')
aspirations = mlines.Line2D([], [], color='black', linestyle="--", label='Dynamic lambda and aspirations')
plt.legend(handles=[clean, aspirations])
plt.show()
