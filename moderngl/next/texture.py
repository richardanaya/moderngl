from typing import Any, Tuple


class Texture:
    __slots__ = ['__mglo', '__level', '__layer', '__swizzle', 'size', 'extra']

    def __init__(self):
        self.__mglo = None  # type: Any
        self.__level = None  # type: int
        self.__layer = None  # type: int
        self.__swizzle = None  # type: str
        self.size = None  # type: Union[Tuple[int, int], Tuple[int, int, int]]
        self.extra = None  # type: Any

    @property
    def swizzle(self):
        return self.__swizzle

    @swizzle.setter
    def swizzle(self, value):
        self.__mglo.swizzle = value

    def level(self, level) -> 'Texture':
        return self.__mglo.level(level)

    def layer(self, layer) -> 'Texture':
        return self.__mglo.layer(layer)

    def write(self, data, viewport=None, alignment=1) -> None:
        self.__mglo.write(data, viewport, alignment, self.__level)

    def read(self, alignment=1, np=False) -> None:
        return self.__mglo.read(self.__level, self.__layer, alignment, np)

    def build_mipmaps(self, base=0, max=-1) -> None:
        self.__mglo.build_mipmaps(base, max)

    def bind_to_image(self, binding, access, format) -> None:
        self.__mglo.bind(binding, access, format)
