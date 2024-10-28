import { Construct } from "constructs";
import { TerraformStack, TerraformVariable, S3Backend, TerraformOutput } from "cdktf";
import { AwsProvider } from "@cdktf/provider-aws/lib/provider";
import { configureWebsite } from "../resources/websiteBucket";
import { configureSSLCertificate } from "../resources/sslCertificate";
import { configureCloudFront } from "../resources/cloudfrontDistribution";
import { configureRoute53 } from "../resources/route53";

function configureWebsiteResources(
  stack: TerraformStack,
  websiteBucketName: TerraformVariable,
  domainName: TerraformVariable,
) {
  const resourceTags = {
    Terraform: "true",
    ComputableCoffee: "true",
  };
  const staticWebsiteStorageBucket = configureWebsite(
    stack,
    websiteBucketName,
    resourceTags,
  );
  const certificateValidation = configureSSLCertificate(
    stack,
    domainName,
    resourceTags,
  );
  const distribution = configureCloudFront(
    stack,
    staticWebsiteStorageBucket,
    certificateValidation,
    domainName,
    resourceTags,
  );
  // output distribution id
  new TerraformOutput(stack, "distribution_id", {
    value: distribution.id,
  });
  configureRoute53(stack, distribution, domainName);
}

export class WebsiteStack extends TerraformStack {
  constructor(scope: Construct, id: string) {
    super(scope, id);

    // Setup the AWS provider
    new AwsProvider(this, "AWS", {
      region: "us-east-1",
      profile: process.env.AWS_PROFILE,
    });

    // Configure Website Resources
    const websiteBucketName = new TerraformVariable(this, "websiteBucketName", {
      type: "string",
      description: "The name of the bucket for the static website",
      nullable: false,
    });
    const domainName = new TerraformVariable(this, "domainName", {
      type: "string",
      description: "The website's domain name",
      nullable: false,
    });
    configureWebsiteResources(this, websiteBucketName, domainName);

    new S3Backend(this, {
      bucket: "cdktf-backend-state-184077986974",
      key: `computable.coffee/terraform.tfstate`,
      encrypt: true,
      region: "us-east-1",
      dynamodbTable: "cdktf-backend-lock",
      profile: process.env.AWS_PROFILE,
    });
  }
}
