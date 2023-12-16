import matplotlib.pyplot as plt
import pandas as pd
import glob

# Leer el archivo CSV con todos los puntos del QuadTree
all_points_data = pd.read_csv("all_points.csv", header=None, names=["X", "Y"])

# Leer el archivo CSV con todos los puntos de los clusters
all_clusters_data = pd.read_csv("all_clusters.csv",
                                header=None,
                                names=["X", "Y", "Cluster"])

# Graficar todos los puntos del QuadTree en gris
plt.scatter(all_points_data["X"],
            all_points_data["Y"],
            color='gray',
            label="All Points")

# Graficar los puntos de todos los clusters en colores diferentes
for cluster_num in all_clusters_data["Cluster"].unique():
    cluster_points = all_clusters_data[all_clusters_data["Cluster"] ==
                                       cluster_num]
    plt.scatter(cluster_points["X"],
                cluster_points["Y"],
                label=f"Cluster {cluster_num}")

# Añadir etiquetas y leyenda
plt.xlabel('X')
plt.ylabel('Y')
plt.legend()
plt.show()
