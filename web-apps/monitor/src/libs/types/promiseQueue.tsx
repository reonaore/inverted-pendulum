export class PromiseQueue<T> {
  private queue: T[] = [];
  private resolvers: ((value: T) => void)[] = [];

  clear = () => {
    this.queue = [];
    this.resolvers = [];
  };

  send = (value: T) => {
    if (this.resolvers.length > 0) {
      const resolve = this.resolvers.shift()!;
      resolve(value);
    } else {
      this.queue.push(value);
    }
  };

  receive = (): Promise<T> => {
    return new Promise<T>((resolve) => {
      if (this.queue.length > 0) {
        resolve(this.queue.shift()!);
      } else {
        this.resolvers.push(resolve);
      }
    });
  };
}
