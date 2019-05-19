package fr.marcsworld.chopstickstracker.model

interface Rectangular {
    val x: Int
    val y: Int
    val width: Int
    val height: Int

    companion object {
        fun getBoundingBox(rect1: Rectangular, rect2: Rectangular): Rectangle {
            val x1 = Math.min(rect1.x, rect2.x)
            val y1 = Math.min(rect1.y, rect2.y)
            val x2 = Math.max(rect1.x + rect1.width, rect2.x + rect2.width)
            val y2 = Math.max(rect1.y + rect1.height, rect2.y + rect2.height)
            return Rectangle(x1, y1, x2 - x1, y2 - y1)
        }

        fun getIntersection(rect1: Rectangular, rect2: Rectangular): Rectangle {
            val x1 = Math.max(rect1.x, rect2.x)
            val y1 = Math.max(rect1.y, rect2.y)
            val x2 = Math.min(rect1.x + rect1.width, rect2.x + rect2.width)
            val y2 = Math.min(rect1.y + rect1.height, rect2.y + rect2.height)

            return if (x1 < x2 && y1 < y2) {
                Rectangle(x1, y1, x2 - x1, y2 - y1)
            } else {
                Rectangle(0, 0, 0, 0)
            }
        }
    }

    fun getArea(): Int {
        return this.width * this.height
    }

    fun isOverlappingWith(other: Rectangular): Boolean {
        return this.x < other.x + other.width && this.x + this.width > other.x &&
                this.y < other.y + other.height && this.y + this.height > other.y
    }
}

