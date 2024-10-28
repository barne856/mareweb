import { App } from "cdktf";
import { WebsiteStack } from "./stacks/WebsiteStack";
import "dotenv/config";

const app = new App();
new WebsiteStack(app, "computable-coffee");
app.synth();
